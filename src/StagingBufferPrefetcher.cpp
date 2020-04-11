#include <iostream>
#include <chrono>
#include <thread>
#include "../include/StagingBufferPrefetcher.h"

StagingBufferPrefetcher::StagingBufferPrefetcher(char *staging_buffer,
                                                 unsigned long long int buffer_size,
                                                 int node_id,
                                                 Sampler* sampler,
                                                 StorageBackend* backend,
                                                 PrefetcherBackend** pf_backends,
                                                 MetadataStore* metadata_store) {
    this->buffer_size = buffer_size;
    this->staging_buffer = staging_buffer;
    this->node_id = node_id;
    this->sampler = new Sampler(*sampler);
    this->backend = backend;
    this->pf_backends = pf_backends;
    this->metadata_store = metadata_store;
}

StagingBufferPrefetcher::~StagingBufferPrefetcher() {
    delete sampler;
}

void StagingBufferPrefetcher::prefetch() {
    //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    for (int i = prefetch_batch; i < sampler->epochs; i++) {
        std::vector<int> curr_access_string;
        sampler->get_node_access_string(node_id, &curr_access_string);
        for (int j = prefetch_offset; j < curr_access_string.size(); j++) {
            int file_id = curr_access_string[j];
            unsigned long file_size = backend->get_file_size(file_id);
            std::string label = backend->get_label(file_id);
            unsigned long entry_size = file_size + label.size() + 1;
            std::unique_lock<std::mutex> read_offset_lock(read_offset_mutex);
            while (staging_buffer_pointer < read_offset && staging_buffer_pointer + entry_size >= read_offset) {
                // Prevent overwriting of non-read data
                std::cout << "Sleeping 1... " << read_offset << ", " << staging_buffer_pointer << std::endl;
                read_offset_cond_var.wait(read_offset_lock);
            }

            if (staging_buffer_pointer + entry_size > buffer_size) {
                // Start again at beginning of array
                staging_buffer_pointer = 0;
                // Ensure that overwriting is not possible after reset of pointer
                while (staging_buffer_pointer + entry_size >= read_offset) {
                    std::cout << "Sleeping 2... " << read_offset << ", " << staging_buffer_pointer << std::endl;
                    std::cout << "Sleeping 2... " << entry_size << std::endl;
                    read_offset_cond_var.wait(read_offset_lock);
                }
            }
            read_offset_lock.unlock();
            std::cout << "Storing at " << staging_buffer_pointer << std::endl;

            strcpy(staging_buffer + staging_buffer_pointer, label.c_str());
            fetch(file_id, staging_buffer + staging_buffer_pointer + label.size() + 1);
            std::unique_lock<std::mutex> staging_buffer_lock(staging_buffer_mutex);
            file_ends.push_back(staging_buffer_pointer + entry_size);
            staging_buffer_pointer += entry_size;
            staging_buffer_cond_var.notify_one();
            staging_buffer_lock.unlock();

            prefetch_offset += 1;
        }
        sampler->advance_batch();
        prefetch_batch += 1;
        prefetch_offset = 0;
    }
}

void StagingBufferPrefetcher::fetch(int file_id, char *dst) {
    int storage_level = metadata_store->get_storage_level(file_id);
    if (storage_level == 0) {
        std::cout << "Fetching from PFS, file id: " << file_id << std::endl;
        backend->fetch(file_id, dst);
    } else {
        std::cout << "Fetching from local storage level " << storage_level << std::endl;
        pf_backends[storage_level - 1]->fetch(file_id, dst);
    }
}

void StagingBufferPrefetcher::advance_read_offset(unsigned long long int new_offset) {
    std::unique_lock<std::mutex> lock(read_offset_mutex);
    read_offset = new_offset;
    read_offset_cond_var.notify_one();
}

unsigned long long int StagingBufferPrefetcher::get_next_file_end() {
    std::unique_lock<std::mutex> staging_buffer_lock(staging_buffer_mutex);
    while (file_ends.empty()) {
        staging_buffer_cond_var.wait(staging_buffer_lock);
    }
    unsigned long long int file_end = file_ends.front();
    file_ends.pop_front();
    return file_end;
}

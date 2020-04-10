#include <iostream>
#include <chrono>
#include <thread>
#include "../include/StagingBufferPrefetcher.h"

StagingBufferPrefetcher::StagingBufferPrefetcher(char *staging_buffer,
                                                 unsigned long long int buffer_size,
                                                 int node_id,
                                                 std::deque<unsigned long long int> *file_ends,
                                                 std::mutex* staging_buffer_mutex,
                                                 std::condition_variable* staging_buffer_cond_var,
                                                 Sampler* sampler,
                                                 StorageBackend* backend,
                                                 PrefetcherBackend** pf_backends,
                                                 MetadataStore* metadata_store) {
    this->buffer_size = buffer_size;
    this->staging_buffer = staging_buffer;
    this->node_id = node_id;
    this->file_ends = file_ends;
    this->staging_buffer_mutex = staging_buffer_mutex;
    this->staging_buffer_cond_var = staging_buffer_cond_var;
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
            if (staging_buffer_pointer < read_offset && staging_buffer_pointer + entry_size > read_offset) {
                // TODO: Prevent overwriting of non-read data
            }

            if (staging_buffer_pointer + entry_size > buffer_size) {
                // Start again at beginning of array
                staging_buffer_pointer = 0;
                // Ensure that overwriting is not possible after reset of pointer
                if (staging_buffer_pointer < read_offset && staging_buffer_pointer + entry_size > read_offset) {

                }
            }

            strcpy(staging_buffer + staging_buffer_pointer, label.c_str());
            fetch(file_id, staging_buffer + staging_buffer_pointer + label.size() + 1);
            std::unique_lock<std::mutex> lock(*staging_buffer_mutex);
            file_ends->push_back(staging_buffer_pointer + entry_size);
            staging_buffer_cond_var->notify_one();
            lock.unlock();
            staging_buffer_pointer += entry_size;

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

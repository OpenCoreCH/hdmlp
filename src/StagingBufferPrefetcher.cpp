#include <iostream>
#include <chrono>
#include <thread>
#include "../include/StagingBufferPrefetcher.h"

StagingBufferPrefetcher::StagingBufferPrefetcher(char *staging_buffer,
                                                 unsigned long long int buffer_size,
                                                 int node_id,
                                                 std::deque<int>* file_ends,
                                                 std::mutex* staging_buffer_mutex,
                                                 std::condition_variable* staging_buffer_cond_var,
                                                 Sampler* sampler,
                                                 StorageBackend* backend) {
    this->buffer_size = buffer_size;
    this->staging_buffer = staging_buffer;
    this->node_id = node_id;
    this->file_ends = file_ends;
    this->staging_buffer_mutex = staging_buffer_mutex;
    this->staging_buffer_cond_var = staging_buffer_cond_var;
    this->sampler = new Sampler(*sampler);
    this->backend = backend;
}

StagingBufferPrefetcher::~StagingBufferPrefetcher() {
    delete sampler;
}

void StagingBufferPrefetcher::prefetch() {
    for (int i = prefetch_batch; i < sampler->epochs; i++) {
        std::vector<int> curr_access_string;
        sampler->get_node_access_string(node_id, &curr_access_string);
        for (int j = prefetch_offset; j < curr_access_string.size(); j++) {
            int file_id = curr_access_string[j];
            unsigned long size = backend->get_entry_size(file_id);
            if (staging_buffer_pointer < read_offset && staging_buffer_pointer + size > read_offset) {
                // TODO: Prevent overwriting of non-read data
            }

            if (staging_buffer_pointer + size > buffer_size) {
                // Start again at beginning of array
                staging_buffer_pointer = 0;
                // Ensure that overwriting is not possible after reset of pointer
                if (staging_buffer_pointer < read_offset && staging_buffer_pointer + size > read_offset) {

                }
            }

            backend->fetch(file_id, staging_buffer + staging_buffer_pointer, size);
            std::unique_lock<std::mutex> lock(*staging_buffer_mutex);
            file_ends->push_back(staging_buffer_pointer + size);
            staging_buffer_cond_var->notify_one();
            lock.unlock();
            staging_buffer_pointer += size;

            prefetch_offset += 1;
        }
        sampler->advance_batch();
        prefetch_batch += 1;
        prefetch_offset = 0;
    }
}

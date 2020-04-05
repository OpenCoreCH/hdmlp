#include <iostream>
#include <chrono>
#include <thread>
#include "../include/StagingBufferPrefetcher.h"

StagingBufferPrefetcher::StagingBufferPrefetcher(char *staging_buffer,
                                                 int buffer_size,
                                                 int node_id,
                                                 std::deque<int>* file_ends,
                                                 Sampler* sampler,
                                                 StorageBackend* backend) {
    this->buffer_size = buffer_size;
    this->staging_buffer = staging_buffer;
    this->node_id = node_id;
    this->file_ends = file_ends;
    this->sampler = new Sampler(*sampler);
    this->backend = backend;
}

StagingBufferPrefetcher::~StagingBufferPrefetcher() {
    delete sampler;
}

void StagingBufferPrefetcher::prefetch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    for (int i = prefetch_batch; i < sampler->epochs; i++) {
        std::vector<int> curr_access_string;
        sampler->get_node_access_string(node_id, &curr_access_string);
        for (int j = prefetch_offset; j < curr_access_string.size(); j++) {
            int file_id = curr_access_string[j];
            int file_size = backend->get_file_size(file_id);
            if (staging_buffer_pointer < read_offset && staging_buffer_pointer + file_size > read_offset) {
                // TODO: Prevent overwriting of non-read data
            }

            if (staging_buffer_pointer + file_size > buffer_size) {
                // Start again at beginning of array
                staging_buffer_pointer = 0;
                // Ensure that overwriting is not possible after reset of pointer
                if (staging_buffer_pointer < read_offset && staging_buffer_pointer + file_size > read_offset) {

                }
            }

            file_ends->push_back(staging_buffer_pointer + file_size);
            backend->fetch(file_id, staging_buffer + staging_buffer_pointer, file_size);
            staging_buffer_pointer += file_size;

            prefetch_offset += 1;
        }
        sampler->advance_batch();
        prefetch_batch += 1;
        prefetch_offset = 0;
    }
}

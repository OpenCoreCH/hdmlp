#include <iostream>
#include "../include/StagingBufferPrefetcher.h"

StagingBufferPrefetcher::StagingBufferPrefetcher(char *staging_buffer,
                                                 int buffer_size,
                                                 int node_id,
                                                 std::vector<int>* file_ends,
                                                 Sampler sampler,
                                                 StorageBackend* backend) {
    this->buffer_size = buffer_size;
    this->staging_buffer = staging_buffer;
    this->node_id = node_id;
    this->file_ends = file_ends;
    this->sampler = &sampler;
    this->backend = backend;
}

void StagingBufferPrefetcher::prefetch() {
    std::vector<int> curr_access_string;
    sampler->get_node_access_string(node_id, &curr_access_string);
    for (int i = prefetch_offset; i < curr_access_string.size(); i++) {
        int file_id = curr_access_string[i];
        int file_size = backend->get_file_size(file_id);
        if (staging_buffer_pointer + file_size < buffer_size) {
            file_ends->push_back(staging_buffer_pointer + file_size);
            backend->fetch(file_id, staging_buffer + staging_buffer_pointer, file_size);
            staging_buffer_pointer += file_size;
        }
    }
}

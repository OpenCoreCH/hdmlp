#include <iostream>
#include "../include/MemoryPrefetcher.h"

MemoryPrefetcher::MemoryPrefetcher(const std::map<std::string, std::string> &backend_options,
                                   const std::vector<int>::const_iterator prefetch_start,
                                   const std::vector<int>::const_iterator prefetch_end,
                                   unsigned long long int capacity,
                                   StorageBackend* backend) {
    buffer = new char[capacity];
    this->prefetch_start = prefetch_start;
    this->prefetch_end = prefetch_end;
    this->backend = backend;
    num_elems = std::distance(prefetch_start, prefetch_end);
    file_ends = new unsigned long long int[num_elems];
}

MemoryPrefetcher::~MemoryPrefetcher() {
    delete[] buffer;
    delete[] file_ends;
}

void MemoryPrefetcher::prefetch() {
    std::cout << "MemoryPrefetcher::prefetch called" << std::endl;
    for (auto ptr = prefetch_start; ptr < prefetch_end; ptr++) {
        int offset = std::distance(prefetch_start, ptr);
        int file_id = *ptr;
        unsigned long long prev_end = 0;
        if (offset != 0) {
            prev_end = file_ends[offset - 1];
        }
        unsigned long file_size = backend->get_file_size(file_id);
        backend->fetch(file_id, buffer + prev_end);
        std::cout << "Storing at" << prev_end << std::endl;
        buffer_offsets[file_id] = offset;
        file_ends[offset] = prev_end + file_size;
        //std::cout << *ptr << std::endl;
    }
}

void MemoryPrefetcher::fetch(int file_id, char *dst) {
    int offset = buffer_offsets[file_id];
    unsigned long long start;
    unsigned long long end = file_ends[offset];
    if (offset == 0) {
        start = 0;
    } else {
        start = file_ends[offset - 1];
    }
    memcpy(dst, buffer + start, end - start);
}

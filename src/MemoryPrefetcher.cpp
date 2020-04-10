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
        std::cout << *ptr << std::endl;
    }
}

#include <iostream>
#include "../include/MemoryPrefetcher.h"

MemoryPrefetcher::MemoryPrefetcher(const std::map<std::string, std::string> &backend_options,
                                   const std::vector<int>::const_iterator prefetch_start,
                                   const std::vector<int>::const_iterator prefetch_end,
                                   unsigned long long int capacity,
                                   StorageBackend* backend) {
    for (auto ptr = prefetch_start; ptr < prefetch_end; ptr++) {
        std::cout << *ptr << std::endl;
    }
}

void MemoryPrefetcher::prefetch() {
    std::cout << "MemoryPrefetcher::prefetch called" << std::endl;
}

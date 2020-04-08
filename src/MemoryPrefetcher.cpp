#include <iostream>
#include "../include/MemoryPrefetcher.h"

MemoryPrefetcher::MemoryPrefetcher(const std::map<std::string, std::string> &backend_options) {

}

void MemoryPrefetcher::prefetch() {
    std::cout << "MemoryPrefetcher::prefetch called" << std::endl;
}

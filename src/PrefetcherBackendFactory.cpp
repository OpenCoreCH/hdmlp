#include "../include/PrefetcherBackendFactory.h"
#include "../include/MemoryPrefetcher.h"
#include "../include/FileSystemPrefetcher.h"

PrefetcherBackend * PrefetcherBackendFactory::create(const std::string &prefetcher_backend,
                                                     const std::map<std::string, std::string> &backend_options,
                                                     unsigned long long int capacity,
                                                     std::vector<int>::const_iterator start,
                                                     std::vector<int>::const_iterator end,
                                                     StorageBackend* storage_backend) {
    PrefetcherBackend* pfb;
    if (prefetcher_backend == "memory") {
        pfb = new MemoryPrefetcher(backend_options, start, end, capacity, storage_backend);
    } else if (prefetcher_backend == "filesystem") {
        pfb = new FileSystemPrefetcher(backend_options, start, end, capacity, storage_backend);
    } else {
        throw std::runtime_error("Unsupported prefetch backend");
    }
    return pfb;
}

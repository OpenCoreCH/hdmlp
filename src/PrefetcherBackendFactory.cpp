#include <iostream>
#include "../include/PrefetcherBackendFactory.h"
#include "../include/MemoryPrefetcher.h"
#include "../include/FileSystemPrefetcher.h"

PrefetcherBackend* PrefetcherBackendFactory::create(const std::string &prefetcher_backend,
                                                    std::map<std::string, std::string> &backend_options,
                                                    unsigned long long int capacity,
                                                    std::vector<int>::const_iterator start,
                                                    std::vector<int>::const_iterator end,
                                                    StorageBackend* storage_backend, MetadataStore* metadata_store,
                                                    int storage_level,
                                                    int no_threads,
                                                    int job_id) {
    PrefetcherBackend* pfb;
    if (prefetcher_backend == "memory") {
        pfb = new MemoryPrefetcher(backend_options, start, end, capacity, storage_backend, metadata_store,
                                   storage_level, true);
    } else if (prefetcher_backend == "filesystem") {
        pfb = new FileSystemPrefetcher(backend_options, start, end, capacity, storage_backend, metadata_store, storage_level, job_id);
    } else {
        throw std::runtime_error("Unsupported prefetch backend");
    }
    return pfb;
}

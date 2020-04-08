#include "../include/PrefetcherBackendFactory.h"
#include "../include/MemoryPrefetcher.h"
#include "../include/FileSystemPrefetcher.h"

PrefetcherBackend* PrefetcherBackendFactory::create(const std::string &backend,
                                                    const std::map<std::string, std::string>& backend_options) {
    PrefetcherBackend* pfb;
    if (backend == "memory") {
        pfb = new MemoryPrefetcher(backend_options);
    } else if (backend == "filesystem") {
        pfb = new FileSystemPrefetcher(backend_options);
    } else {
        throw std::runtime_error("Unsupported prefetch backend");
    }
    return pfb;
}

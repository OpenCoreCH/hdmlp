#ifndef HDMLP_PREFETCHERBACKENDFACTORY_H
#define HDMLP_PREFETCHERBACKENDFACTORY_H


#include <string>
#include <vector>
#include <map>
#include "PrefetcherBackend.h"
#include "StorageBackend.h"
#include "MetadataStore.h"

class PrefetcherBackendFactory {
public:
    static PrefetcherBackend*
    create(const std::string &prefetcher_backend,
           std::map<std::string, std::string> &backend_options,
           unsigned long long int capacity,
           std::vector<int>::const_iterator start,
           std::vector<int>::const_iterator end,
           StorageBackend* storage_backend,
           MetadataStore* metadata_store,
           int storage_level);
};


#endif //HDMLP_PREFETCHERBACKENDFACTORY_H

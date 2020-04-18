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
           std::vector<int>::iterator start,
           std::vector<int>::iterator end,
           StorageBackend* storage_backend,
           MetadataStore* metadata_store, int storage_level, int job_id,
           int node_id);
};


#endif //HDMLP_PREFETCHERBACKENDFACTORY_H

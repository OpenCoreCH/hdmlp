#ifndef HDMLP_DISTRIBUTEDSTORE_H
#define HDMLP_DISTRIBUTEDSTORE_H


#include "MetadataStore.h"
#include "PrefetcherBackend.h"

class DistributedStore {
public:
    explicit DistributedStore(MetadataStore* metadata_store);
    ~DistributedStore();

    void set_prefetcher_backends(PrefetcherBackend** prefetcher_backends);
    int get_no_nodes();
    int get_node_id();

private:
    PrefetcherBackend** pf_backends = nullptr;
    MetadataStore* metadata_store;
    int n{};
    int node_id{};
};


#endif //HDMLP_DISTRIBUTEDSTORE_H

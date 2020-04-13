#ifndef HDMLP_DISTRIBUTEDMANAGER_H
#define HDMLP_DISTRIBUTEDMANAGER_H


#include "MetadataStore.h"
#include "PrefetcherBackend.h"
#include "StorageBackend.h"
#include <vector>

class DistributedManager {
public:
    DistributedManager(MetadataStore* metadata_store, StorageBackend* storage_backend);
    ~DistributedManager();

    void set_prefetcher_backends(PrefetcherBackend** prefetcher_backends);
    int get_no_nodes();
    int get_node_id();
    void serve();
    bool fetch(int file_id, char* dst);
    void distribute_prefetch_strings(std::vector<int>* local_prefetch_string,
                                     std::vector<std::vector<int>::const_iterator>* storage_class_ends,
                                     int num_storage_classes);
    int generate_and_broadcast_seed();

private:
    struct FileAvailability {
        int node_id;
        int storage_class;
        int offset;
    };
    PrefetcherBackend** pf_backends = nullptr;
    MetadataStore* metadata_store;
    StorageBackend* storage_backend;
    int n;
    int node_id;
    std::unordered_map<int, FileAvailability> file_availability; // Stores mapping of file_id -> availability info

    void parse_received_prefetch_data(int* rcv_data, int arr_size, int global_max_size);
};


#endif //HDMLP_DISTRIBUTEDMANAGER_H

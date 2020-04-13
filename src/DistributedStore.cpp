#include <mpi.h>
#include "../include/DistributedStore.h"

DistributedStore::DistributedStore(MetadataStore* metadata_store) { // NOLINT(cppcoreguidelines-pro-type-member-init)
    this->metadata_store = metadata_store;
    int provided;
    MPI_Init_thread(nullptr, nullptr, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE) {
        throw std::runtime_error("Implementation doesn't support MPI_THREAD_MULTIPLE");
    }
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &node_id);
}

DistributedStore::~DistributedStore() {
    MPI_Finalize();
}

void DistributedStore::set_prefetcher_backends(PrefetcherBackend** prefetcher_backends) {
    pf_backends = prefetcher_backends;
}

int DistributedStore::get_no_nodes() {
    return n;
}

int DistributedStore::get_node_id() {
    return node_id;
}

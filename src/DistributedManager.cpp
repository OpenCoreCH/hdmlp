#include <mpi.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "../include/DistributedManager.h"

DistributedManager::DistributedManager(MetadataStore* metadata_store, StorageBackend* storage_backend) { // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    this->metadata_store = metadata_store;
    this->storage_backend = storage_backend;
    int provided;
    MPI_Init_thread(nullptr, nullptr, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE) {
        throw std::runtime_error("Implementation doesn't support MPI_THREAD_MULTIPLE");
    }
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &node_id);
}

DistributedManager::~DistributedManager() {
    MPI_Finalize();
}

void DistributedManager::set_prefetcher_backends(PrefetcherBackend** prefetcher_backends) {
    pf_backends = prefetcher_backends;
}

int DistributedManager::get_no_nodes() {
    return n;
}

int DistributedManager::get_node_id() {
    return node_id;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void DistributedManager::serve() {
    while (true) {
        int req_file_id;
        MPI_Status status;
        MPI_Recv(&req_file_id,1, MPI_INT, MPI_ANY_SOURCE, MPI_TAG_UB, MPI_COMM_WORLD, &status);
        int source = status.MPI_SOURCE;
        int storage_level = metadata_store->get_storage_level(req_file_id);
        if (storage_level > 0) {
            unsigned long len;
            char* loc = pf_backends[storage_level]->get_location(req_file_id, &len);
            MPI_Send(loc, (int) len, MPI_CHAR, source, req_file_id, MPI_COMM_WORLD);
        } else {
            // Send zero bytes to indicate that file isn't available
            MPI_Send(nullptr, 0, MPI_CHAR, source, req_file_id, MPI_COMM_WORLD);
        }
    }
}
#pragma clang diagnostic pop

bool DistributedManager::fetch(int file_id, char* dst) {
    unsigned long file_size = storage_backend->get_file_size(file_id);
    int from_node = 0;
    MPI_Send(&file_id, 1, MPI_INT, from_node, MPI_TAG_UB, MPI_COMM_WORLD);
    MPI_Status response_status;
    MPI_Recv(dst, (int) file_size, MPI_CHAR, from_node, file_id, MPI_COMM_WORLD, &response_status);
    int response_length;
    MPI_Get_count(&response_status, MPI_CHAR, &response_length);
    return response_length > 0;
}

void DistributedManager::distribute_prefetch_strings(std::vector<int>* local_prefetch_string,
                                                     std::vector<std::vector<int>::const_iterator>* storage_class_ends,
                                                     int num_storage_classes) {
    int local_size = local_prefetch_string->size();
    int global_max_size;
    MPI_Allreduce(&local_size, &global_max_size, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    // We send at most (num_storage_classes - 1) length values, total size (including used number of storage classes) therefore
    int arr_size = global_max_size + num_storage_classes;
    int send_data[arr_size];
    int rcv_data[n * arr_size];
    for (int i = 0; i < local_size; i++) {
        send_data[i] = (*local_prefetch_string)[i];
    }
    std::vector<int>::const_iterator prev_end = local_prefetch_string->begin();
    // Store number of elements per storage class in send_data
    send_data[global_max_size] = storage_class_ends->size();
    for (int i = 0; i < storage_class_ends->size(); i++) {
        send_data[global_max_size + i + 1] = std::distance(prev_end, (*storage_class_ends)[i]);
        prev_end = (*storage_class_ends)[i];
    }
    MPI_Datatype arr_type;
    MPI_Type_contiguous(arr_size, MPI_INT, &arr_type);
    MPI_Type_commit(&arr_type);
    MPI_Allgather(&send_data, 1, arr_type, rcv_data, 1, arr_type, MPI_COMM_WORLD);
    parse_received_prefetch_data(rcv_data, arr_size, global_max_size);
    std::this_thread::sleep_for(std::chrono::milliseconds(node_id * 1000));
    for (auto elem : file_availability) {
        std::cout << "File id " << elem.first << " available at node " << elem.second.node_id << " (offset " <<
        elem.second.offset << ") in storage class " << elem.second.storage_class << std::endl;
    }

}

void DistributedManager::parse_received_prefetch_data(int* rcv_data, int arr_size, int global_max_size) {
    for (int i = 0; i < n; i++) {
        if (i != node_id) {
            int offset = i * arr_size;
            int used_storage_classes = rcv_data[offset + global_max_size];
            std::vector<int> elems_per_storage_class;
            for (int j = offset + global_max_size + 1; j < offset + global_max_size + 1 + used_storage_classes; j++) {
                elems_per_storage_class.push_back(rcv_data[j]);
            }
            for (int j = 0; j < elems_per_storage_class.size(); j++) {
                int storage_class_elems = elems_per_storage_class[j];
                for (int k = offset; k < offset + storage_class_elems; k++) {
                    int file_id = rcv_data[k];
                    struct FileAvailability file_avail{};
                    file_avail.node_id = i;
                    file_avail.offset = k - offset;
                    file_avail.storage_class = j + 1;
                    if (file_availability.count(file_id) > 0) {
                        if (file_avail.storage_class < file_availability[file_id].storage_class) {
                            file_availability[file_id] = file_avail;
                        }
                    } else {
                        file_availability[file_id] = file_avail;
                    }
                }
                offset += storage_class_elems;
            }
        }
    }
}

int DistributedManager::generate_and_broadcast_seed() {
    int seed;
    if (node_id == 0) {
        seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    }
    MPI_Bcast(&seed, 1, MPI_INT, 0, MPI_COMM_WORLD);
    return seed;
}


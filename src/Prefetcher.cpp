#include <iostream>
#include "../include/Prefetcher.h"
#include "../include/FileSystemBackend.h"
#include "../include/Configuration.h"
#include "../include/StagingBufferPrefetcher.h"


Prefetcher::Prefetcher(const std::wstring& dataset_path,
                       int batch_size,
                       int epochs,
                       int distr_scheme,
                       bool drop_last_batch,
                       int seed) {
    int n = 1;
    int node_id = 0;
    backend = new FileSystemBackend(dataset_path);
    sampler = new Sampler(backend->get_length(), n, batch_size, epochs, distr_scheme, drop_last_batch, seed);
    Configuration config("../../cpp/hdmlp/data/hdmlp.cfg");
    config.get_storage_classes(&capacities, &threads, &bandwidths);
    config.get_pfs_bandwidth(&pfs_bandwidth);
    int staging_buffer_capacity = capacities[0] * 1024 * 1024;
    staging_buffer = new char[staging_buffer_capacity];

    StagingBufferPrefetcher sbf(staging_buffer,
                                staging_buffer_capacity,
                                node_id,
                                &file_ends,
                                sampler,
                                backend);
    sbf.prefetch();
}

int Prefetcher::get_next_file_end() {
    int file_end;
    if (!file_ends.empty()) {
        file_end = file_ends.front();
        file_ends.pop_front();
    } else {
        throw std::runtime_error("Empty buffer");
    }
    return file_end;
}

char *Prefetcher::get_staging_buffer() {
    return staging_buffer;
}

Prefetcher::~Prefetcher() {
    delete backend;
    delete sampler;
    delete[] staging_buffer;
}

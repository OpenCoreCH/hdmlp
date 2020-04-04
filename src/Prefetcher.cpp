#include <iostream>
#include "../include/Prefetcher.h"
#include "../include/FileSystemBackend.h"
#include "../include/Configuration.h"


Prefetcher::Prefetcher(const std::wstring& dataset_path,
                       int batch_size,
                       int epochs,
                       int distr_scheme,
                       bool drop_last_batch,
                       int seed) {
    backend = new FileSystemBackend(dataset_path);
    sampler = new Sampler(backend->get_length(), 1, batch_size, epochs, distr_scheme, drop_last_batch, seed);
    Configuration config("../../cpp/hdmlp/data/hdmlp.cfg");
    config.get_storage_classes(&capacities, &threads, &bandwidths);
    config.get_pfs_bandwidth(&pfs_bandwidth);
    int staging_buffer_capacity = capacities[0];
    staging_buffer = new char[staging_buffer_capacity];
}

char *Prefetcher::get_staging_buffer() {
    return staging_buffer;
}

Prefetcher::~Prefetcher() {
    delete backend;
    delete sampler;
    delete staging_buffer;
}

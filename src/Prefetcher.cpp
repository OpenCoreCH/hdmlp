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
    backend = new FileSystemBackend(dataset_path);
    sampler = new Sampler(backend->get_length(), n, batch_size, epochs, distr_scheme, drop_last_batch, seed);
    Configuration config("../../cpp/hdmlp/data/hdmlp.cfg");
    config.get_storage_classes(&capacities, &no_threads, &bandwidths);
    config.get_pfs_bandwidth(&pfs_bandwidth);
    int staging_buffer_capacity = capacities[0] * 1024 * 1024;
    staging_buffer = new char[staging_buffer_capacity];
    sbf = new StagingBufferPrefetcher(staging_buffer,
                                      staging_buffer_capacity,
                                      node_id,
                                      &file_ends,
                                      sampler,
                                      backend);
    init_threads();
}

void Prefetcher::init_threads() {
    for (int j = 0; j < no_threads.size(); j++) {
        int no_storage_class_threads = no_threads[j];
        std::vector<std::thread> storage_class_threads;
        for (int k = 0; k < no_storage_class_threads; k++) {
            if (j == 0) {
                std::thread thread(&StagingBufferPrefetcher::prefetch, std::ref(*sbf));
                storage_class_threads.push_back(std::move(thread));
            }
        }
        threads.push_back(std::move(storage_class_threads));
    }
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
    delete sbf;
    delete[] staging_buffer;
}

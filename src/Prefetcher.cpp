#include <iostream>
#include "../include/Prefetcher.h"
#include "../include/FileSystemBackend.h"
#include "../include/Configuration.h"
#include "../include/StagingBufferPrefetcher.h"
#include "../include/PrefetcherBackendFactory.h"


Prefetcher::Prefetcher(const std::wstring& dataset_path, // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
                       int batch_size,
                       int epochs,
                       int distr_scheme,
                       bool drop_last_batch,
                       int seed) {
    int n = 2;
    backend = new FileSystemBackend(dataset_path);
    sampler = new Sampler(backend, n, batch_size, epochs, distr_scheme, drop_last_batch, seed);
    metadata_store = new MetadataStore;
    init_config();
    init_threads();
}

void Prefetcher::init_config() {
    Configuration config("/Volumes/GoogleDrive/Meine Ablage/Dokumente/1 - Schule/1 - ETHZ/6. Semester/Bachelor Thesis/hdmlp/cpp/hdmlp/data/hdmlp.cfg");
    config.get_storage_classes(&config_capacities, &config_no_threads, &config_bandwidths, &config_pf_backends, &config_pf_backend_options);
    config.get_pfs_bandwidth(&config_pfs_bandwidth);
}

void Prefetcher::init_threads() {
    int classes = config_no_threads.size();
    pf_backends = new PrefetcherBackend*[classes - 1];
    std::vector<std::vector<int>::const_iterator> storage_class_ends;
    sampler->get_prefetch_string(node_id, &config_capacities, &prefetch_string, &storage_class_ends);
    for (int j = 0; j < classes; j++) {
        if (j > storage_class_ends.size()) {
            // Only create thread if there is something to prefetch
            continue;
        }
        int no_storage_class_threads = config_no_threads[j];
        std::vector<std::thread> storage_class_threads;
        for (int k = 0; k < no_storage_class_threads; k++) {
            if (j == 0) {
                unsigned long long int staging_buffer_capacity = config_capacities[0];
                staging_buffer = new char[staging_buffer_capacity];
                sbf = new StagingBufferPrefetcher(staging_buffer,
                                                  staging_buffer_capacity,
                                                  node_id,
                                                  &file_ends,
                                                  &staging_buffer_mutex,
                                                  &staging_buffer_cond_var,
                                                  sampler,
                                                  backend,
                                                  pf_backends,
                                                  metadata_store);
                std::thread thread(&StagingBufferPrefetcher::prefetch, std::ref(*sbf));
                storage_class_threads.push_back(std::move(thread));
            } else {
                std::vector<int>::const_iterator prefetch_start;
                if (j == 1) {
                    prefetch_start = prefetch_string.begin();
                } else {
                    prefetch_start = storage_class_ends[j - 2];
                }
                auto prefetch_end = storage_class_ends[j - 1];
                PrefetcherBackend* pf = PrefetcherBackendFactory::create(config_pf_backends[j - 1],
                                                                         config_pf_backend_options[j - 1],
                                                                         config_capacities[j],
                                                                         prefetch_start,
                                                                         prefetch_end,
                                                                         backend,
                                                                         metadata_store,
                                                                         j);
                pf_backends[j - 1] = pf;
                std::thread thread(&PrefetcherBackend::prefetch, std::ref(*pf));
                storage_class_threads.push_back(std::move(thread));
            }
        }
        threads.push_back(std::move(storage_class_threads));
    }
}

unsigned long long int Prefetcher::get_next_file_end() {
    std::unique_lock<std::mutex> lock(staging_buffer_mutex);
    while (file_ends.empty()) {
        staging_buffer_cond_var.wait(lock);
    }
    unsigned long long int file_end = file_ends.front();
    file_ends.pop_front();
    return file_end;
}

int Prefetcher::get_dataset_length() {
    return backend->get_length();
}

char *Prefetcher::get_staging_buffer() {
    return staging_buffer;
}

Prefetcher::~Prefetcher() {
    int used_classes = threads.size();
    for (auto &thread_list : threads) {
        for (auto &thread : thread_list) {
            thread.join();
        }
    }
    for (int i = 0; i < used_classes - 1; i++) {
        delete pf_backends[i];
    }
    delete[] pf_backends;
    delete backend;
    delete sampler;
    delete metadata_store;
    delete sbf;
    delete[] staging_buffer;
}


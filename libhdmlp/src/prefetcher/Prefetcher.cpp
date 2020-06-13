#include <codecvt>
#include <locale>
#include "../../include/prefetcher/Prefetcher.h"
#include "../../include/storage/FileSystemBackend.h"
#include "../../include/utils/Configuration.h"
#include "../../include/prefetcher/StagingBufferPrefetcher.h"
#include "../../include/prefetcher/PrefetcherBackendFactory.h"


Prefetcher::Prefetcher(const std::wstring& dataset_path, // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
                       const std::wstring& config_path,
                       int batch_size,
                       int epochs,
                       int distr_scheme,
                       bool drop_last_batch,
                       int seed,
                       int job_id,
                       wchar_t** transform_names,
                       char* transform_args,
                       int transform_output_size,
                       int transform_len) {
    init_config(config_path);
    metadata_store = new MetadataStore(networkbandwidth_clients, networkbandwidth_filesystem, &config_pfs_bandwidth, &config_bandwidths,
                                       &config_no_threads);
    distr_manager = new DistributedManager(metadata_store, pf_backends);
    n = distr_manager->get_no_nodes();
    metadata_store->set_no_nodes(n);
    node_id = distr_manager->get_node_id();
    backend = new FileSystemBackend(dataset_path, checkpoint, checkpoint_path, node_id);
    distr_manager->set_backend(backend);
    this->job_id = job_id;
    if (seed == 0) {
        seed = distr_manager->generate_and_broadcast_seed();
    }
    sampler = new Sampler(backend, n, batch_size, epochs, distr_scheme, drop_last_batch, seed);
    sampler->get_prefetch_string(node_id, &config_capacities, &prefetch_string, &storage_class_ends, true);
    distr_manager->distribute_prefetch_strings(&prefetch_string, &storage_class_ends, config_no_threads.size());
    if (transform_len > 0) {
        transform_pipeline = new TransformPipeline*[config_no_threads[0]];
        for (int i = 0; i < config_no_threads[0]; i++) {
            transform_pipeline[i] = new TransformPipeline(transform_names, transform_args, transform_output_size, transform_len);
        }
    }
    init_threads();
    init_distr_threads();
}

void Prefetcher::init_config(const std::wstring& path) {
    using type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<type, wchar_t> converter;
    std::string str_path = converter.to_bytes(path);
    Configuration config(str_path);
    config.get_storage_classes(&config_capacities, &config_no_threads, &config_bandwidths, &config_pf_backends, &config_pf_backend_options);
    config.get_pfs_bandwidth(&config_pfs_bandwidth);
    no_distributed_threads = config.get_no_distributed_threads();
    config.get_bandwidths(&networkbandwidth_clients, &networkbandwidth_filesystem);
    int classes = config_no_threads.size();
    pf_backends = new PrefetcherBackend* [classes - 1];
    for (int i = 0; i < classes - 1; i++) {
        pf_backends[i] = nullptr;
    }
    checkpoint = config.get_checkpoint();
    if (checkpoint) {
        checkpoint_path = config.get_checkpoint_path();
    }
}

void Prefetcher::init_threads() {
    threads.resize(storage_class_ends.size() + 1);
    for (int j = storage_class_ends.size(); j >= 0; j--) {
        int no_storage_class_threads = config_no_threads[j];
        std::vector<std::thread> storage_class_threads;
        for (int k = 0; k < no_storage_class_threads; k++) {
            if (j == 0) {
                if (k == 0) {
                    unsigned long long int staging_buffer_capacity = config_capacities[0];
                    staging_buffer = new char[staging_buffer_capacity];
                    int transform_output_size = 0;
                    if (transform_pipeline != nullptr) {
                        transform_output_size = transform_pipeline[0]->get_output_size();
                    }
                    sbf = new StagingBufferPrefetcher(staging_buffer,
                                                      staging_buffer_capacity,
                                                      node_id,
                                                      no_storage_class_threads,
                                                      sampler,
                                                      backend,
                                                      pf_backends,
                                                      metadata_store,
                                                      distr_manager,
                                                      transform_pipeline,
                                                      transform_output_size);
                }
                std::thread thread(&StagingBufferPrefetcher::prefetch, std::ref(*sbf), k);
                storage_class_threads.push_back(std::move(thread));
            } else {
                if (k == 0) {
                    std::vector<int>::iterator prefetch_start;
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
                                                                             j,
                                                                             job_id,
                                                                             node_id);
                    pf_backends[j - 1] = pf;
                }
                std::thread thread(&PrefetcherBackend::prefetch, std::ref(*pf_backends[j - 1]));
                storage_class_threads.push_back(std::move(thread));
            }
        }
        threads[j] = std::move(storage_class_threads);
    }
}

void Prefetcher::init_distr_threads() {
    for (int i = 0; i < no_distributed_threads; i++) {
        std::thread thread(&DistributedManager::serve, std::ref(*distr_manager));
        distr_threads.push_back(std::move(thread));
    }
}

unsigned long long int Prefetcher::get_next_file_end() {
    return sbf->get_next_file_end();
}

void Prefetcher::notify_data_consumed(unsigned long long int until_offset) {
    sbf->advance_read_offset(until_offset);
}

int Prefetcher::get_dataset_length() {
    return backend->get_length();
}

char* Prefetcher::get_staging_buffer() {
    return staging_buffer;
}

Prefetcher::~Prefetcher() {
    int used_classes = threads.size();
    for (auto& thread_list : threads) {
        for (auto& thread : thread_list) {
            thread.join();
        }
    }
    distr_manager->stop_all_threads(distr_threads.size());
    for (auto& distr_thread : distr_threads) {
        distr_thread.join();
    }
    for (int i = 0; i < used_classes - 1; i++) {
        delete pf_backends[i];
    }
    if (transform_pipeline != nullptr) {
        for (int i = 0; i < config_no_threads[0]; i++) {
            if (transform_pipeline[i] != nullptr) {
                delete transform_pipeline[i];
            }
        }
        delete transform_pipeline;
    }
    delete[] pf_backends;
    delete backend;
    delete sampler;
    delete metadata_store;
    delete distr_manager;
    delete sbf;
    delete[] staging_buffer;
}

int Prefetcher::get_node_id() {
    return node_id;
}

int Prefetcher::get_no_nodes() {
    return n;
}


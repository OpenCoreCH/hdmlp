#ifndef HDMLP_PREFETCHER_H
#define HDMLP_PREFETCHER_H

#include <string>
#include <deque>
#include <thread>
#include "StorageBackend.h"
#include "Sampler.h"
#include "StagingBufferPrefetcher.h"
#include "PrefetcherBackend.h"
#include "MetadataStore.h"

class Prefetcher {
public:
    Prefetcher(const std::wstring& dataset_path,
               int batch_size,
               int epochs,
               int distr_scheme,
               bool drop_last_batch,
               int seed);
    ~Prefetcher();
    char* get_staging_buffer();
    unsigned long long int get_next_file_end();
    void notify_data_consumed(unsigned long long int until_offset);
    int get_dataset_length();

private:
    char* staging_buffer;
    StorageBackend* backend;
    Sampler* sampler;
    StagingBufferPrefetcher* sbf;
    PrefetcherBackend** pf_backends;
    MetadataStore* metadata_store;
    std::vector<int> prefetch_string;
    std::vector<unsigned long long int> config_capacities;
    std::vector<int> config_no_threads;
    std::vector<std::string> config_pf_backends;
    std::vector<std::map<std::string, std::string>> config_pf_backend_options;
    std::vector<std::map<int, int>> config_bandwidths;
    std::map<int, int> config_pfs_bandwidth;
    std::deque<unsigned long long int> file_ends;
    std::vector<std::vector<std::thread>> threads;
    int node_id = 0;

    void init_config();
    void init_threads();
};


#endif //HDMLP_PREFETCHER_H

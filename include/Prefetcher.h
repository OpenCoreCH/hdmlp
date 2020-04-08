#ifndef HDMLP_PREFETCHER_H
#define HDMLP_PREFETCHER_H

#include <string>
#include <deque>
#include <thread>
#include "StorageBackend.h"
#include "Sampler.h"
#include "StagingBufferPrefetcher.h"
#include "PrefetcherBackend.h"

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
    int get_next_file_end();
    int get_dataset_length();

private:
    char* staging_buffer;
    StorageBackend* backend;
    Sampler* sampler;
    StagingBufferPrefetcher* sbf;
    PrefetcherBackend** pf_backends{};
    std::mutex staging_buffer_mutex;
    std::condition_variable staging_buffer_cond_var;
    std::vector<int> config_capacities;
    std::vector<int> config_no_threads;
    std::vector<std::string> config_pf_backends;
    std::vector<std::map<std::string, std::string>> config_pf_backend_options;
    std::vector<std::map<int, int>> config_bandwidths;
    std::map<int, int> config_pfs_bandwidth;
    std::deque<int> file_ends;
    std::vector<std::vector<std::thread>> threads;
    int node_id = 0;

    void init_config();
    void init_threads();
};


#endif //HDMLP_PREFETCHER_H

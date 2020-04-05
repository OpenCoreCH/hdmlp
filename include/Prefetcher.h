#ifndef HDMLP_PREFETCHER_H
#define HDMLP_PREFETCHER_H

#include <string>
#include <deque>
#include "StorageBackend.h"
#include "Sampler.h"

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

private:
    char* staging_buffer;
    StorageBackend* backend;
    Sampler* sampler;
    std::vector<int> capacities;
    std::vector<int> threads;
    std::vector<std::map<int, int>> bandwidths;
    std::map<int, int> pfs_bandwidth;
    std::deque<int> file_ends;

};


#endif //HDMLP_PREFETCHER_H

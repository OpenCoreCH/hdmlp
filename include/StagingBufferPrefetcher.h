#ifndef HDMLP_STAGINGBUFFERPREFETCHER_H
#define HDMLP_STAGINGBUFFERPREFETCHER_H
#include <deque>
#include "Sampler.h"
#include "PrefetcherBackend.h"


class StagingBufferPrefetcher {
public:
    StagingBufferPrefetcher(char* staging_buffer,
                            unsigned long long int buffer_size,
                            int node_id,
                            std::deque<unsigned long long int> *file_ends,
                            std::mutex* staging_buffer_mutex,
                            std::condition_variable* staging_buffer_cond_var,
                            Sampler* sampler,
                            StorageBackend* backend,
                            PrefetcherBackend** pf_backends);
    ~StagingBufferPrefetcher();
    void prefetch();

private:
    unsigned long long int buffer_size;
    int prefetch_batch = 0;
    int prefetch_offset = 0;
    unsigned long long int staging_buffer_pointer = 0;
    int read_offset = 0;
    char* staging_buffer;
    std::deque<unsigned long long int>* file_ends;
    std::mutex* staging_buffer_mutex;
    std::condition_variable* staging_buffer_cond_var;
    Sampler* sampler;
    StorageBackend* backend;
    PrefetcherBackend** pf_backends;
    int node_id;
};


#endif //HDMLP_STAGINGBUFFERPREFETCHER_H

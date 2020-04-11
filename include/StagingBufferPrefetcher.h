#ifndef HDMLP_STAGINGBUFFERPREFETCHER_H
#define HDMLP_STAGINGBUFFERPREFETCHER_H
#include <deque>
#include "Sampler.h"
#include "PrefetcherBackend.h"
#include "MetadataStore.h"


class StagingBufferPrefetcher {
public:
    StagingBufferPrefetcher(char* staging_buffer,
                            unsigned long long int buffer_size,
                            int node_id,
                            Sampler* sampler,
                            StorageBackend* backend,
                            PrefetcherBackend** pf_backends,
                            MetadataStore* metadata_store);
    ~StagingBufferPrefetcher();
    void prefetch();
    void advance_read_offset(unsigned long long int new_offset);
    unsigned long long int get_next_file_end();

private:
    unsigned long long int buffer_size;
    int prefetch_batch = 0;
    int prefetch_offset = 0;
    unsigned long long int staging_buffer_pointer = 0;
    unsigned long long int read_offset = 0;
    char* staging_buffer;
    std::deque<unsigned long long int> file_ends;
    std::mutex staging_buffer_mutex;
    std::mutex read_offset_mutex;
    std::condition_variable staging_buffer_cond_var;
    std::condition_variable read_offset_cond_var;
    Sampler* sampler;
    StorageBackend* backend;
    PrefetcherBackend** pf_backends;
    MetadataStore* metadata_store;
    int node_id;

    void fetch(int file_id, char *dst);
};


#endif //HDMLP_STAGINGBUFFERPREFETCHER_H

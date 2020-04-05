#ifndef HDMLP_STAGINGBUFFERPREFETCHER_H
#define HDMLP_STAGINGBUFFERPREFETCHER_H
#include <deque>
#include "Sampler.h"


class StagingBufferPrefetcher {
public:
    StagingBufferPrefetcher(char* staging_buffer,
                            int buffer_size,
                            int node_id,
                            std::deque<int>* file_ends,
                            Sampler* sampler,
                            StorageBackend* backend);
    ~StagingBufferPrefetcher();
    void prefetch();

private:
    int buffer_size;
    int prefetch_batch = 0;
    int prefetch_offset = 0;
    int staging_buffer_pointer = 0;
    int read_offset = 0;
    char* staging_buffer;
    std::deque<int>* file_ends;
    Sampler* sampler;
    StorageBackend* backend;
    int node_id;
};


#endif //HDMLP_STAGINGBUFFERPREFETCHER_H

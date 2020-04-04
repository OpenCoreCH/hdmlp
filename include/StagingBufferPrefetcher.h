#ifndef HDMLP_STAGINGBUFFERPREFETCHER_H
#define HDMLP_STAGINGBUFFERPREFETCHER_H
#include <vector>
#include "Sampler.h"


class StagingBufferPrefetcher {
public:
    StagingBufferPrefetcher(char* staging_buffer,
                            int buffer_size,
                            int node_id,
                            std::vector<int>* file_ends,
                            Sampler sampler,
                            StorageBackend* backend);
    void prefetch();

private:
    int buffer_size;
    int prefetch_batch = 0;
    int prefetch_offset = 0;
    int staging_buffer_pointer = 0;
    int read_offset = 0;
    char* staging_buffer;
    std::vector<int>* file_ends;
    Sampler* sampler;
    StorageBackend* backend;
    int node_id;
};


#endif //HDMLP_STAGINGBUFFERPREFETCHER_H

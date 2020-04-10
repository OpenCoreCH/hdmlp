#ifndef HDMLP_MEMORYPREFETCHER_H
#define HDMLP_MEMORYPREFETCHER_H


#include <map>
#include <string>
#include <vector>
#include "PrefetcherBackend.h"
#include "StorageBackend.h"
#include "MetadataStore.h"

class MemoryPrefetcher : public PrefetcherBackend {
public:
    MemoryPrefetcher(std::map<std::string, std::string> &backend_options,
                              std::vector<int>::const_iterator prefetch_start,
                              std::vector<int>::const_iterator prefetch_end,
                              unsigned long long int capacity, StorageBackend* backend, MetadataStore* metadata_store,
                              int storage_level, bool alloc_buffer);
    ~MemoryPrefetcher() override;
    void prefetch() override;
    void fetch(int file_id, char *dst) override;

protected:
    char* buffer;
    unsigned long long int* file_ends;
    std::map<int, int> buffer_offsets;
    StorageBackend* backend;
    MetadataStore* metadata_store;
    std::vector<int>::const_iterator prefetch_start;
    std::vector<int>::const_iterator prefetch_end;
    int num_elems;
    int storage_level;
    unsigned long long capacity;
    bool buffer_allocated = false;
};


#endif //HDMLP_MEMORYPREFETCHER_H

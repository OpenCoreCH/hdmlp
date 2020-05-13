#ifndef HDMLP_MEMORYPREFETCHER_H
#define HDMLP_MEMORYPREFETCHER_H


#include <map>
#include <string>
#include <vector>
#include "PrefetcherBackend.h"
#include "../storage/StorageBackend.h"
#include "../utils/MetadataStore.h"

class MemoryPrefetcher : public PrefetcherBackend {
public:
    MemoryPrefetcher(std::map<std::string, std::string>& backend_options,
                     std::vector<int>::iterator prefetch_start,
                     std::vector<int>::iterator prefetch_end,
                     unsigned long long int capacity, StorageBackend* backend, MetadataStore* metadata_store,
                     int storage_level, bool alloc_buffer);

    ~MemoryPrefetcher() override;

    void prefetch() override;

    void fetch(int file_id, char* dst) override;

    char* get_location(int file_id, unsigned long* len) override;

    int get_prefetch_offset() override;

    bool is_done() override;

protected:
    char* buffer;
    unsigned long long int* file_ends;
    std::unordered_map<int, int> buffer_offsets;
    StorageBackend* backend;
    MetadataStore* metadata_store;
    std::vector<int>::iterator prefetch_start;
    std::vector<int>::iterator prefetch_end;
    std::mutex prefetcher_mutex;
    int num_elems;
    int prefetch_offset = 0;
    int storage_level;
    unsigned long long capacity;
    bool buffer_allocated = false;

    unsigned long long get_file_start(int file_id);
};


#endif //HDMLP_MEMORYPREFETCHER_H

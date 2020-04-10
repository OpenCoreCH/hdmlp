#ifndef HDMLP_MEMORYPREFETCHER_H
#define HDMLP_MEMORYPREFETCHER_H


#include <map>
#include <string>
#include <vector>
#include "PrefetcherBackend.h"
#include "StorageBackend.h"

class MemoryPrefetcher : public PrefetcherBackend {
public:
    explicit MemoryPrefetcher(const std::map<std::string, std::string> &backend_options,
                              std::vector<int>::const_iterator prefetch_start,
                              std::vector<int>::const_iterator prefetch_end,
                              unsigned long long int capacity,
                              StorageBackend* backend);
    ~MemoryPrefetcher() override;
    void prefetch() override;

private:
    char* buffer;
    unsigned long long int* file_ends;
    std::vector<int>::const_iterator prefetch_start;
    std::vector<int>::const_iterator prefetch_end;
    int num_elems;
};


#endif //HDMLP_MEMORYPREFETCHER_H

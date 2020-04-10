#ifndef HDMLP_MEMORYPREFETCHER_H
#define HDMLP_MEMORYPREFETCHER_H


#include <map>
#include <string>
#include <vector>
#include "PrefetcherBackend.h"

class MemoryPrefetcher : public PrefetcherBackend {
public:
    explicit MemoryPrefetcher(const std::map<std::string, std::string> &backend_options,
                              std::vector<int>::const_iterator prefetch_start,
                              std::vector<int>::const_iterator prefetch_end);
    void prefetch() override;
};


#endif //HDMLP_MEMORYPREFETCHER_H

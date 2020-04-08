#ifndef HDMLP_MEMORYPREFETCHER_H
#define HDMLP_MEMORYPREFETCHER_H


#include <map>
#include <string>
#include "PrefetcherBackend.h"

class MemoryPrefetcher : public PrefetcherBackend {
public:
    explicit MemoryPrefetcher(const std::map<std::string, std::string>& backend_options);
};


#endif //HDMLP_MEMORYPREFETCHER_H

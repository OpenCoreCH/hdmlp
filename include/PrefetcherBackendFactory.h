#ifndef HDMLP_PREFETCHERBACKENDFACTORY_H
#define HDMLP_PREFETCHERBACKENDFACTORY_H


#include <string>
#include <vector>
#include <map>
#include "PrefetcherBackend.h"

class PrefetcherBackendFactory {
public:
    static PrefetcherBackend *
    create(const std::string &backend, const std::map<std::string, std::string> &backend_options,
           std::vector<int>::const_iterator start,
           std::vector<int>::const_iterator end);
};


#endif //HDMLP_PREFETCHERBACKENDFACTORY_H

#ifndef HDMLP_PREFETCHERBACKENDFACTORY_H
#define HDMLP_PREFETCHERBACKENDFACTORY_H


#include <string>
#include <map>
#include "PrefetcherBackend.h"

class PrefetcherBackendFactory {
public:
    static PrefetcherBackend *create(const std::string &backend, const std::map<std::string, std::string>& backend_options);
};


#endif //HDMLP_PREFETCHERBACKENDFACTORY_H

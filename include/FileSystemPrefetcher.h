#ifndef HDMLP_FILESYSTEMPREFETCHER_H
#define HDMLP_FILESYSTEMPREFETCHER_H


#include <map>
#include <string>
#include "PrefetcherBackend.h"

class FileSystemPrefetcher : public PrefetcherBackend {
public:
    explicit FileSystemPrefetcher(const std::map<std::string, std::string>& backend_options);
    void prefetch() override;
};


#endif //HDMLP_FILESYSTEMPREFETCHER_H

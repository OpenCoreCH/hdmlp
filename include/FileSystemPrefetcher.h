#ifndef HDMLP_FILESYSTEMPREFETCHER_H
#define HDMLP_FILESYSTEMPREFETCHER_H


#include <map>
#include <string>
#include <vector>
#include "PrefetcherBackend.h"
#include "StorageBackend.h"

class FileSystemPrefetcher : public PrefetcherBackend {
public:
    explicit FileSystemPrefetcher(const std::map<std::string, std::string> &backend_options,
                                  std::vector<int>::const_iterator prefetch_start,
                                  std::vector<int>::const_iterator prefetch_end,
                                  unsigned long long int capacity,
                                  StorageBackend* backend);
    void prefetch() override;
};


#endif //HDMLP_FILESYSTEMPREFETCHER_H

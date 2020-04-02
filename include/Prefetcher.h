#ifndef HDMLP_PREFETCHER_H
#define HDMLP_PREFETCHER_H

#include <string>
#include "StorageBackend.h"
#include "Sampler.h"

class Prefetcher {
public:
    Prefetcher(const std::wstring& dataset_path,
               int batch_size,
               int distr_scheme,
               bool drop_last_batch,
               int seed);
private:
    StorageBackend* backend;
    Sampler* sampler;
};


#endif //HDMLP_PREFETCHER_H

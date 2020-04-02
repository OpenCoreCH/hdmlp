#include "../include/Prefetcher.h"
#include "../include/FileSystemBackend.h"

Prefetcher::Prefetcher(const std::wstring& dataset_path,
                       int batch_size,
                       int distr_scheme,
                       bool drop_last_batch,
                       int seed) {
    backend = new FileSystemBackend(dataset_path);
    sampler = new Sampler(backend->get_length(), 1, batch_size, distr_scheme, drop_last_batch, seed);
}

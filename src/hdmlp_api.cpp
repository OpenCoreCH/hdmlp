#include "../include/FileSystemBackend.h"
#include "../include/Prefetcher.h"

extern "C"
#include "../include/hdmlp_api.h"

#include <iostream>

void setup(wchar_t * dataset_path,
           int batch_size,
           int distr_scheme,
           bool drop_last_batch,
           int seed) {
    Prefetcher(dataset_path,
               batch_size,
               distr_scheme,
               drop_last_batch,
               seed);
}

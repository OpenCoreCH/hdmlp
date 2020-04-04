#include "../include/FileSystemBackend.h"
#include "../include/Prefetcher.h"

extern "C"
#include "../include/hdmlp_api.h"

#include <iostream>

char* setup(wchar_t * dataset_path,
           int batch_size,
           int epochs,
           int distr_scheme,
           bool drop_last_batch,
           int seed) {
    Prefetcher(dataset_path,
               batch_size,
               epochs,
               distr_scheme,
               drop_last_batch,
               seed);
    char* test = new char[1024*1024];
    for (int i = 0; i < 20; i++) {
        test[i] = 'a';
    }
    return test;
}

#include "../include/FileSystemBackend.h"
#include "../include/Prefetcher.h"
#include "../include/hdmlp_api.h"

Prefetcher* pf;
char* setup(wchar_t * dataset_path,
           int batch_size,
           int epochs,
           int distr_scheme,
           bool drop_last_batch,
           int seed) {
    pf = new Prefetcher(dataset_path,
               batch_size,
               epochs,
               distr_scheme,
               drop_last_batch,
               seed);
    char* staging_buffer = pf->get_staging_buffer();
    return staging_buffer;
}

int get_next_file_end() {
    return pf->get_next_file_end();
}

void destroy() {
    delete pf;
}
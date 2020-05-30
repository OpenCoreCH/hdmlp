#include <iostream>
#include "../../include/storage/FileSystemBackend.h"
#include "../../include/prefetcher/Prefetcher.h"
#include "../../include/utils/hdmlp_api.h"


int setup(wchar_t* dataset_path,
          wchar_t* config_path,
          int batch_size,
          int epochs,
          int distr_scheme,
          bool drop_last_batch,
          int seed,
          wchar_t** transform_names,
          char* transform_args,
          int transform_output_size,
          int transform_len) {
    printf("HDMLP API %p\n", (void *)transform_args);
    int job_id = 0;
    while (job_id < PARALLEL_JOBS_LIMIT) {
        if (!used_map[job_id]) {
            used_map[job_id] = true;
            break;
        } else {
            job_id++;
        }
    }
    if (job_id == PARALLEL_JOBS_LIMIT) {
        throw std::runtime_error("Maximal parallel jobs exceeded");
    }
    pf[job_id] = new Prefetcher(dataset_path,
                                config_path,
                                batch_size,
                                epochs,
                                distr_scheme,
                                drop_last_batch,
                                seed,
                                job_id,
                                transform_names,
                                transform_args,
                                transform_output_size,
                                transform_len);
    return job_id;
}

char* get_staging_buffer(int job_id) {
    char* staging_buffer = pf[job_id]->get_staging_buffer();
    return staging_buffer;
}

int get_node_id(int job_id) {
    return pf[job_id]->get_node_id();
}

int get_no_nodes(int job_id) {
    return pf[job_id]->get_no_nodes();
}

int length(int job_id) {
    return pf[job_id]->get_dataset_length();
}

unsigned long long int get_next_file_end(int job_id) {
    pf[job_id]->notify_data_consumed(consumed_until[job_id]);
    unsigned long long int file_end = pf[job_id]->get_next_file_end();
    consumed_until[job_id] = file_end;
    return file_end;
}

void destroy(int job_id) {
    delete pf[job_id];
    used_map[job_id] = false;
}
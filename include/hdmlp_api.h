#ifndef HDMLP_HDMLP_API_H
#define HDMLP_HDMLP_API_H
#define PARALLEL_JOBS_LIMIT 255
extern "C" {
    Prefetcher* pf[PARALLEL_JOBS_LIMIT];
    bool used_map[PARALLEL_JOBS_LIMIT] = { false };

    int setup(wchar_t * dataset_path,
              int batch_size,
              int epochs,
              int distr_scheme,
              bool drop_last_batch,
              int seed);

    char* get_staging_buffer(int job_id);

    int length(int job_id);

    unsigned long long int get_next_file_end(int job_id);

    void destroy(int job_id);
};
#endif //HDMLP_HDMLP_API_H

#ifndef HDMLP_HDMLP_API_H
#define HDMLP_HDMLP_API_H
extern "C" {
    char* setup(wchar_t * dataset_path,
                int batch_size,
                int epochs,
                int distr_scheme,
                bool drop_last_batch,
                int seed);

    int length();

    int get_next_file_end();

    void destroy();
};
#endif //HDMLP_HDMLP_API_H

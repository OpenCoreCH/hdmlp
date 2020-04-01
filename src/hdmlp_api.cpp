extern "C"
#include "../include/hdmlp_api.h"

#include <iostream>

void setup(wchar_t * dataset_path,
           int batch_size,
           int distr_scheme,
           bool drop_last_batch,
           int seed) {
    std::wcout << dataset_path << std::endl;
    std::cout << batch_size << std::endl;
    std::cout << distr_scheme << std::endl;
    std::cout << drop_last_batch << std::endl;
    std::cout << seed << std::endl;
}

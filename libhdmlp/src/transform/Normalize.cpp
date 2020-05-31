#include "../../include/transform/Normalize.h"
#include <torch/csrc/api/include/torch/data/transforms/tensor.h>

char* Normalize::parse_arguments(char* arg_array) {
    double* args = ((double*) arg_array);
    mean[0] = *args;
    mean[1] = *(args + 1);
    mean[2] = *(args + 2);
    std[0] = *(args + 3);
    std[1] = *(args + 4);
    std[2] = *(args + 5);
    return arg_array + 6 * sizeof(double);
}

at::Tensor Normalize::transform(const at::Tensor& tensor) {
    return torch::data::transforms::Normalize<>(mean, std)(tensor);
}

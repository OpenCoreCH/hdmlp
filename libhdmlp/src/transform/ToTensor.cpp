#include "../../include/transform/ToTensor.h"
#include <torch/csrc/autograd/generated/variable_factories.h>

at::Tensor ToTensor::transform(const cv::Mat& img) {
    at::Tensor tensor_img;
    cv::Mat float_img;
    img.convertTo(float_img, CV_32F, 1./255.);
    tensor_img = torch::from_blob(float_img.data, {4, img.rows, img.cols, 3}, at::kFloat);
    tensor_img = tensor_img.permute({0, 3, 1, 2});
    // Tensor is backed by cv::Mat data field that will get deallocated after function has exited. By calling contiguous at this point, we ensure data is copied (because of the previous permutation)
    return tensor_img.contiguous();
}

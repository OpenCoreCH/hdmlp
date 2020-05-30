#include <codecvt>
#include "../../include/transform/TransformPipeline.h"
#include "../../include/transform/Transformation.h"
#include "../../include/transform/ImgDecode.h"
#include "../../include/transform/Resize.h"
#include "../../include/transform/ToTensor.h"


TransformPipeline::TransformPipeline(wchar_t** transform_names, char* transform_args, int transform_output_size, int transform_len) {
    output_size = transform_output_size;
    for (int i = 0; i < transform_len; i++) {
        using type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<type, wchar_t> converter;
        std::string transform_name = converter.to_bytes(transform_names[i]);
        Transformation* transform;
        if (transform_name == "ImgDecode") {
            transform = new ImgDecode();
        } else if (transform_name == "Resize") {
            Resize* resize = new Resize();
            transform_args = resize->parse_arguments(transform_args);
            transform = (Transformation*) resize;
        } else if (transform_name == "ToTensor") {
            transform = new ToTensor();
        } else {
            throw std::runtime_error("Transformation not implemented");
        }
        transformations.push_back(transform);
        transformation_names.push_back(transform_name);
    }
}

void TransformPipeline::transform(char* src_buffer, unsigned long src_len, char* dst_buffer) {
    cv::Mat img;
    at::Tensor tensor;
    auto transformation_pointer = transformations.begin();
    for (auto const &transform_name : transformation_names) {
        if (transform_name == "ImgDecode") {
            ImgDecode* transform = (ImgDecode*) (*transformation_pointer);
            img = transform->transform(src_buffer, src_len);
        } else if (transform_name == "Resize") {
            Resize* transform = (Resize*) (*transformation_pointer);
            transform->transform(img);
        } else if (transform_name == "ToTensor") {
            ToTensor* transform = (ToTensor*) (*transformation_pointer);
            tensor = transform->transform(img);
        }
        transformation_pointer++;
    }
    memcpy(dst_buffer, tensor.data_ptr(), output_size);
}

TransformPipeline::~TransformPipeline() {
    for (auto &transformation : transformations) {
        delete transformation;
    }
}

int TransformPipeline::get_output_size() {
    return output_size;
}

#include <codecvt>
#include "../../include/transform/Transformation.h"
#include "../../include/transform/ImgDecode.h"
#include "../../include/transform/Resize.h"
#include "../../include/transform/ToTensor.h"
#include "../../include/transform/RandomHorizontalFlip.h"
#include "../../include/transform/RandomVerticalFlip.h"
#include "../../include/transform/RandomResizedCrop.h"
#include "../../include/transform/Normalize.h"


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
            transform = new Resize();
        } else if (transform_name == "ToTensor") {
            transform = new ToTensor();
        } else if (transform_name == "RandomHorizontalFlip") {
            transform = new RandomHorizontalFlip();
        } else if (transform_name == "RandomVerticalFlip") {
            transform = new RandomVerticalFlip();
        } else if (transform_name == "RandomResizedCrop") {
            transform = new RandomResizedCrop();
        } else if (transform_name == "Normalize") {
            transform = new Normalize();
        } else {
            throw std::runtime_error("Transformation not implemented");
        }
        transform_args = transform->parse_arguments(transform_args);
        transformations.push_back(transform);
        transformation_names.push_back(transform_name);
    }
}

void TransformPipeline::transform(char* src_buffer, unsigned long src_len, char* dst_buffer) {
    this->src_buffer = src_buffer;
    this->src_len = src_len;
    for (auto const &transformation : transformations) {
        transformation->transform(this);
    }
    memcpy(dst_buffer, img.data, output_size);
}

TransformPipeline::~TransformPipeline() {
    for (auto &transformation : transformations) {
        delete transformation;
    }
}

int TransformPipeline::get_output_size() const {
    return output_size;
}

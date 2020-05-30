#include "../../include/transform/Resize.h"

char* Resize::parse_arguments(char* arg_array) {
    printf("Parse argument %p\n", (void *)arg_array);
    width = *((int*) arg_array);
    height = *((int*) (arg_array + sizeof(int)));
    return arg_array + 2 * sizeof(int);
}

void Resize::transform(cv::Mat img) {
    std::cout << width << std::endl;
    std::cout << height << std::endl;
    cv::resize(img, img, cv::Size(width, height));
}

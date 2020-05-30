#ifndef HDMLP_TRANSFORMATION_H
#define HDMLP_TRANSFORMATION_H
#include <opencv2/opencv.hpp>
#include <ATen/ATen.h>


class Transformation {
public:
    // Parse the required arguments out of the arg_array (pointing to the offset of the transformation, return new pointer containing offset of next transformation)
    char* parse_arguments(char* arg_array);
};


#endif //HDMLP_TRANSFORMATION_H

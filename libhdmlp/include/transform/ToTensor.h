#ifndef HDMLP_TOTENSOR_H
#define HDMLP_TOTENSOR_H


#include "Transformation.h"

class ToTensor : public Transformation {
public:
    at::Tensor transform(const cv::Mat& img);

};


#endif //HDMLP_TOTENSOR_H

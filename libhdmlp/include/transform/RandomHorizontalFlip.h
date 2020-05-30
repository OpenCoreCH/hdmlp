#ifndef HDMLP_RANDOMHORIZONTALFLIP_H
#define HDMLP_RANDOMHORIZONTALFLIP_H


#include "Transformation.h"

class RandomHorizontalFlip : public Transformation {
public:
    char* parse_arguments(char* arg_array);

    void transform(cv::Mat img);

private:
    float p;
};


#endif //HDMLP_RANDOMHORIZONTALFLIP_H

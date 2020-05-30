#ifndef HDMLP_IMGDECODE_H
#define HDMLP_IMGDECODE_H


#include "Transformation.h"

class ImgDecode : public Transformation {
public:
    cv::Mat transform(char* src_buffer, unsigned long src_len);

};


#endif //HDMLP_IMGDECODE_H

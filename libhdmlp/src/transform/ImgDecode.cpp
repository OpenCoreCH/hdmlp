#include "../../include/transform/ImgDecode.h"

cv::Mat ImgDecode::transform(char* src_buffer, unsigned long src_len) {
    cv::Mat img;
    img = cv::imdecode(cv::Mat(1, (int) src_len, CV_8UC1, src_buffer), cv::IMREAD_COLOR);
    return img;
}

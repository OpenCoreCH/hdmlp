#include "../../include/transform/Reshape.h"

void Reshape::transform(TransformPipeline* pipeline) {
    pipeline->img = cv::Mat(1, (int) pipeline->src_len, CV_32FC1, pipeline->src_buffer);
}

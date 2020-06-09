#ifndef HDMLP_TRANSFORMPIPELINE_H
#define HDMLP_TRANSFORMPIPELINE_H


#include <list>
#include <opencv2/opencv.hpp>

class Transformation; // Forward declaration because of circular dependencies

class TransformPipeline {
public:
    TransformPipeline(wchar_t** transform_names, char* transform_args, int transform_output_size, int transform_len);

    ~TransformPipeline();

    void transform(char* src_buffer, unsigned long src_len, char* dst_buffer);

    int get_output_size() const;

    cv::Mat img;

    unsigned long src_len;

    char* src_buffer;

private:
    int output_size;
    std::list<Transformation*> transformations;
    std::list<std::string> transformation_names;
};


#endif //HDMLP_TRANSFORMPIPELINE_H

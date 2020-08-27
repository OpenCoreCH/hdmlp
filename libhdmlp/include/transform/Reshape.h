#ifndef HDMLP_RESHAPE_H
#define HDMLP_RESHAPE_H


#include "Transformation.h"

class Reshape : public Transformation {
public:
    void transform(TransformPipeline* pipeline);
};


#endif //HDMLP_RESHAPE_H

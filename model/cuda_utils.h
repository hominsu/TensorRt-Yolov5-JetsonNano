#ifndef YOLOV5_MODEL_CUDA_UTILS_H_
#define YOLOV5_MODEL_CUDA_UTILS_H_

#include <cuda_runtime_api.h>

#ifndef CUDA_CHECK
#define CUDA_CHECK(callstr)\
    {\
        cudaError_t error_code = callstr;\
        if (error_code != cudaSuccess) {\
            std::cerr << "CUDA error " << error_code << " at " << __FILE__ << ":" << __LINE__;\
            assert(0);\
        }\
    }
#endif  // CUDA_CHECK

#endif  // YOLOV5_MODEL_CUDA_UTILS_H_


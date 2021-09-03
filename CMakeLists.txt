cmake_minimum_required(VERSION 3.10)
project(yolov5)

add_definitions(-std=c++17)
option(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
set(CMAKE_CXX_STANDARD 17)

message("")
message("Operation system is ${CMAKE_SYSTEM}")

# cuda
include_directories(/usr/local/cuda/include)
link_directories(/usr/local/cuda/lib64)
# tensorrt
include_directories(/usr/include/aarch64-linux-gnu/)
link_directories(/usr/lib/aarch64-linux-gnu/)
# other
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Ofast -Wfatal-errors -D_MWAITXINTRIN_H_INCLUDED")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(-DDEBUG)
    message(STATUS "CMake Build Type: Debug")
    message("")
elseif (CMAKE_BUILD_TYPE STREQUAL Release)
    message(STATUS "CMake Build Type: Release")
    message("")
endif ()

file(GLOB ProjectSRC
        "model/*.cc"
        "*.cc")

find_package(CUDA REQUIRED)
find_package(OpenCV)
find_package(Threads REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# 创建静态链接库，也可以生成静态的（SHARED）
cuda_add_library(YoloLayerPlugin STATIC ${PROJECT_SOURCE_DIR}/yololayer.cu)
target_link_libraries(YoloLayerPlugin nvinfer cudart)

add_executable(${PROJECT_NAME} ${ProjectSRC})
target_link_libraries(${PROJECT_NAME} nvinfer cudart YoloLayerPlugin ${OpenCV_LIBS} Threads::Threads)

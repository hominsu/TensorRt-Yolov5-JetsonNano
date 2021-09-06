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
set(CMAKE_PREFIX_PATH "/home/ynu/.local")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(-DDEBUG)
    message(STATUS "CMake Build Type: Debug")
    message("")
elseif (CMAKE_BUILD_TYPE STREQUAL Release)
    message(STATUS "CMake Build Type: Release")
    message("")
endif ()

find_package(CUDA REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#[[
# Proto file
get_filename_component(hw_proto "./proto/detect.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

message("Pos: [${CMAKE_CURRENT_BINARY_DIR}]")
# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/detect.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/detect.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/detect.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/detect.grpc.pb.h")
add_custom_command(
        OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
        DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# hw_grpc_proto
add_library(hw_grpc_proto
        ${hw_grpc_srcs}
        ${hw_grpc_hdrs}
        ${hw_proto_srcs}
        ${hw_proto_hdrs})
        ]]

file(GLOB ProjectSRC
        "detect_service/*.cc"
        "model/*.cc"
        "*.cc")

# 创建静态链接库，也可以生成动态的（SHARED）
cuda_add_library(YoloLayer STATIC model/yololayer.cu)
target_link_libraries(YoloLayer nvinfer cudart)

add_executable(${PROJECT_NAME} ${ProjectSRC})
target_link_libraries(${PROJECT_NAME} nvinfer cudart YoloLayer ${hw_grpc_proto} ${OpenCV_LIBS} Threads::Threads gRPC::grpc++)

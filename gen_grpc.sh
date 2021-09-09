#!/bin/bash

grpc_cpp_plugin_location=$(which grpc_cpp_plugin)

protoc --proto_path=./proto/ --grpc_out=./detect_service --plugin=protoc-gen-grpc="$grpc_cpp_plugin_location" detect.proto
printf "Generate 'detect.proto' message classes finished\n"

protoc --proto_path=./proto/ --cpp_out=./detect_service detect.proto
printf "Generate 'detect.proto' service classes finished\n"

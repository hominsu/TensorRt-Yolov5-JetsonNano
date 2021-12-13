#!/bin/bash
cmake -DCMAKE_BUILD_TYPE=Release ..

cores_num=$(grep -c processor /proc/cpuinfo)

printf "\nUsing %d cores to build ...\n\n" "$cores_num"

cmake --build . -j"$cores_num"
printf "\nBuild finished ...\n\n"
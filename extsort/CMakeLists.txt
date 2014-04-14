project(extsort)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

add_executable(sort extsort.cpp main.cpp)
add_executable(memsort memsort.cpp)

set(CMAKE_CXX_FLAGS "-std=c++11")

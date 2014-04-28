project(HeapDB)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11 -W -Wall -Wextra")

include_directories("BufferManager" "Container")
add_executable(test-buffermanager "BufferManager/main.cpp"
                                  "BufferManager/concurrentlist.cpp"
                                  "BufferManager/concurrentlist_simple.cpp"
                                  "BufferManager/concurrentstack.cpp"
                                  "BufferManager/emptyexception.cpp"
                                  "BufferManager/twoq.cpp")
add_executable(test-hashtable-single "Container/test-hashtable-single.cpp")

# Dummy library to "use" all .cpp files
add_library(dummy "Container/hashtable.cpp")

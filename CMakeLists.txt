project(HeapDB)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11 -pthread -W -Wall -Wextra")

include_directories("BufferManager" "Container")
add_executable(test-twoq "BufferManager/main.cpp"
                         "BufferManager/concurrentlist.cpp"
                         "BufferManager/concurrentlist_simple.cpp"
                         "BufferManager/concurrentstack.cpp"
                         "BufferManager/emptyexception.cpp")
add_executable(test-buffermanager "BufferManager/test-buffermanager.cpp"
                                  "BufferManager/bufferframe.cpp"
                                  "BufferManager/bufferframeinternal.cpp"
                                  "BufferManager/buffermanager.cpp"
                                  "BufferManager/emptyexception.cpp"
                                  "Container/hashtable.cpp")
add_executable(test-buffermanager-concurrent "BufferManager/test-buffermanager-concurrent.cpp"
                                             "BufferManager/bufferframe.cpp"
                                             "BufferManager/bufferframeinternal.cpp"
                                             "BufferManager/buffermanager.cpp"
                                             "BufferManager/emptyexception.cpp"
                                             "Container/hashtable.cpp")
add_executable(test-hashtable-single "Container/test-hashtable-single.cpp"
                                     "Container/hashtable.cpp")

# Dummy library to "use" all .cpp files
add_library(dummy "Container/hashtable-impl.cpp"
                  "BufferManager/twoq.cpp")

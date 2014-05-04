project(HeapDB)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11 -O0 -pthread -W -Wall -Wextra")

include_directories("BufferManager")
add_executable(test-twoq "BufferManager/main.cpp"
                         "BufferManager/concurrentlist.cpp"
                         "BufferManager/concurrentlist_simple.cpp"
                         "BufferManager/concurrentstack.cpp"
                         "BufferManager/emptyexception.cpp")
add_executable(test-buffermanager "BufferManager/test-buffermanager.cpp"
                                  "BufferManager/bufferframe.cpp"
                                  "BufferManager/buffermanager.cpp"
                                  "BufferManager/buffermanagerhashtable.cpp"
                                  "BufferManager/replacementmanager.cpp"
                                  "BufferManager/twoq.cpp"
                                  "BufferManager/emptyexception.cpp")
add_executable(test-buffermanager-concurrent "BufferManager/test-buffermanager-concurrent.cpp"
                                             "BufferManager/bufferframe.cpp"
                                             "BufferManager/buffermanager.cpp"
                                             "BufferManager/buffermanagerhashtable.cpp"
                                             "BufferManager/replacementmanager.cpp"
                                             "BufferManager/twoq.cpp"
                                             "BufferManager/emptyexception.cpp")

# Dummy library to "use" all .cpp files
add_library(dummy "BufferManager/twoq.cpp")

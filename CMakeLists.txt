project(HeapDB)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11 -O0 -pthread -W -Wall -Wextra")


#TODO: correct pathes!
set(PROTOBUF_INCLUDE_DIR "../HeapDB/protobuf-2.5.0/src")
set(PROTOBUF_PROTOC_EXECUTABLE "../HeapDB/protobuf-2.5.0/src/protoc")
find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIRS})



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

add_executable(test-segment-manager "SegmentManager/test-segment-manager.cpp"
                                    "SegmentManager/slottedpage.cpp"
                                    "SegmentManager/spsegment.cpp")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS "SegmentManager/schema.proto")

add_executable(schema-test "SegmentManager/schema-test.cpp" ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(schema-test ${PROTOBUF_LIBRARIES})

# Dummy library to "use" all .cpp files
add_library(dummy "BufferManager/twoq.cpp" )

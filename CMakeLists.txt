project(HeapDB)
cmake_minimum_required(VERSION 2.8)

include(FindProtobuf)

set(CMAKE_CXX_FLAGS "-std=c++11 -O0 -pthread -W -Wall -Wextra")

set(PROTOBUF_PROTOC_EXECUTABLE "/usr/bin/protoc")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS "SegmentManager/schema.proto")

include_directories("gtest"
                    "BufferManager"
                    "Operators"
                    "SegmentManager"
                    "SegmentManager/SlottedPage"
                    "SegmentManager/BPlusTree")

file(GLOB_RECURSE TEST_FILES "test/*.cpp")
file(GLOB_RECURSE BUFFER_MANAGER_FILES "BufferManager/*.cpp")
file(GLOB_RECURSE OPERATORS_FILES "Operators/*.cpp")
file(GLOB_RECURSE SEGMENT_MANAGER_FILES "SegmentManager/*.cpp")
file(GLOB_RECURSE SEGMENT_MANAGER_HEADERS "SegmentManager/*.h")

add_executable(run-all-tests "gtest/gtest_main.cc"
                     "gtest/gtest-all.cc"
                     ${TEST_FILES}
                     ${BUFFER_MANAGER_FILES}
                     ${OPERATORS_FILES}
                     ${SEGMENT_MANAGER_FILES}
                     ${PROTO_SRCS} ${PROTO_HDRS}
)
target_link_libraries(run-all-tests ${PROTOBUF_LIBRARIES})

add_executable(pblus-manual "mains/bplus_manualtest.cpp"
                     ${BUFFER_MANAGER_FILES}
                     ${SEGMENT_MANAGER_FILES}
                     ${PROTO_SRCS} ${PROTO_HDRS}
)
target_link_libraries(pblus-manual ${PROTOBUF_LIBRARIES})

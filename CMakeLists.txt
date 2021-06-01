cmake_minimum_required(VERSION 3.12)
project(mb_opencl)

find_package(OpenCL REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g -pipe")
set(CMAKE_CXX_FLAGS_RELEASE "-s -Os -ffast-math -pipe")

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS src/*.h)

add_executable(mb_opencl main.cpp ${SRCS} ${HDRS})
target_link_libraries(mb_opencl OpenCL::OpenCL)

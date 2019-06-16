cmake_minimum_required(VERSION 3.6)
project(PosixThreadPool)

set(CMAKE_CXX_STANDARD 98)

set(SRC threadPoolExample.cpp ThreadPool.cpp)

add_definitions(-Wall -O2 -DNDEBUG)

add_executable(threadPoolExample ${SRC})

target_link_libraries(threadPoolExample pthread)
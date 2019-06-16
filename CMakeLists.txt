cmake_minimum_required(VERSION 3.6)
project(PosixThreadPool)

set(CMAKE_CXX_STANDARD 98)

include_directories(${PROJECT_SOURCE_DIR})

set(SRC threadPoolExample.cpp src/ThreadPool.cpp)

add_definitions(-Wall -O2 -DNDEBUG)

add_executable(threadPoolExample ${SRC})

target_link_libraries(threadPoolExample pthread)

option(BUILD_TEST "Build gtest for localMinimum project" ON)

if (${BUILD_TEST})
    enable_testing()
    add_subdirectory(test)
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 11)


set(SRC threadPoolTest.cpp ${PROJECT_SOURCE_DIR}/src/ThreadPool.cpp)

add_executable(threadPoolTest ${SRC})

if (${GTEST_FOUND})
    message("Found gtest headers: ${GTEST_INCLUDE_DIRS}")
    message("Found gtest libs: ${GTEST_BOTH_LIBRARIES}")
else()
    message(FATAL_ERROR "Gtest library wasn't found!")
endif()

if (${GMOCK_FOUND})
    message("Found gmock headers: ${GMOCK_INCLUDE_DIRS}")
    message("Found gmock libs: ${GMOCK_BOTH_LIBRARIES}")
else()
    message(FATAL_ERROR "Gmock library wasn't found!")
endif()

target_link_libraries(threadPoolTest ${GTEST_BOTH_LIBRARIES} ${GMOCK_BOTH_LIBRARIES} pthread)

gtest_discover_tests(threadPoolTest)
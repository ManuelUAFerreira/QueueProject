cmake_minimum_required(VERSION 3.10)
project(QueueProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(SOURCE_FILES
    queue_project.hpp
    queue_project.tpp
    queue_project_gtest.cpp
)

add_executable(QueueTests ${SOURCE_FILES})
add_executable(QueueExample main.cpp)

target_link_libraries(QueueTests ${GTEST_LIBRARIES} gtest_main pthread)
target_link_libraries(QueueExample pthread)

enable_testing()

add_test(NAME QueueTests COMMAND QueueTests)

cmake_minimum_required(VERSION 3.15)

project(EventLoop)

add_subdirectory(include)

option(BUILD_EXAMPLES "" FALSE)

if(BUILD_EXAMPLES)
    message("Building examples")
    add_subdirectory(example)
endif()

option(BUILD_TESTS "" FALSE)

if(BUILD_TESTS)
    message("Building tests")
    add_subdirectory(tests)
endif()
cmake_minimum_required(VERSION 3.15)

project(EventLoop)

# Compiller congigs
set(CMAKE_CXX_STANDARD 17)

# Thread configs
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_subdirectory(src)

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
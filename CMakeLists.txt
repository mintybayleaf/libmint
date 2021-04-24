# Root bclib CMakeLists.txt

cmake_minimum_required(VERSION 3.13.4)
set(PROJECT_NAME "mint")
option(BUILD_EXAMPLES "Build examples" OFF)

project(
    "${PROJECT_NAME}"
    VERSION 0.0.1
    LANGUAGES C
)

# Disable insource builds
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In source builds not allowed, set -B and -H")
endif()


# Threading
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


# Build examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

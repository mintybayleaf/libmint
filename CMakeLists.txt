# Root bclib CMakeLists.txt

cmake_minimum_required(VERSION 3.13.4)
set(PROJECT_NAME "mint")
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(USE_SHARED_LIBRARY "Build shared library" OFF)

project(
    "${PROJECT_NAME}"
    VERSION 0.0.1
    LANGUAGES C
)

# Disable insource builds
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In source builds not allowed, set -B and -H")
endif()

if(USE_SHARED_LIBRARY)
    # Add the main library
    add_library("${PROJECT_NAME}" SHARED entry_stub.c)
else()
    add_library("${PROJECT_NAME}" entry_stub.c)
endif()

# ADD HEADERS HERE
list(APPEND MINT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/libs/loggo.h")

# This is public because we want a transitive dep
target_include_directories("${PROJECT_NAME}" PUBLIC $<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs>)

# Threading
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE Threads::Threads)

# Math
target_link_libraries("${PROJECT_NAME}" PRIVATE m)

# Flags
if(MSVC)
    target_compile_options(
        "${PROJECT_NAME}"
        PRIVATE /W4
        $<$<CONFIG:RELEASE>:/O2>
    )
else()
    target_compile_options(
        "${PROJECT_NAME}"
        PRIVATE -Wall
        -Wextra
        -pedantic
        -Werror
        -Wswitch-enum
        -Wcast-align
        -Wpointer-arith
        -Wundef
        -Wnested-externs
        -Wcast-qual
        -Wshadow
        -Wunreachable-code
        -Wfloat-equal
        $<$<CONFIG:RELEASE>:-O2>
    )
endif()

# Debugging symbol set on Debug builds only
target_compile_definitions("${PROJECT_NAME}" PRIVATE $<$<CONFIG:DEBUG>:MINT__DEBUG>)

# Use c11
set_target_properties("${PROJECT_NAME}" PROPERTIES C_STANDARD 11)

# Output properties
set_target_properties("${PROJECT_NAME}"
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install actual library, export target and install it, install headers
install(TARGETS "${PROJECT_NAME}" EXPORT "Mint-export" LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(EXPORT "Mint-export" FILE "MintTargets.cmake" NAMESPACE Mint:: DESTINATION "lib/cmake/Mint")

# 
install(
    FILES "${MINT_HEADERS}"
    DESTINATION "include/mint"
)
install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/MintConfig.cmake"
    DESTINATION "lib/cmake/Mint" 
)

# Build and run tests
if(BUILD_TESTS) 
    add_subdirectory(tests)
endif()


# Build examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
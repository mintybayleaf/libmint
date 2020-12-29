# Examples CMakeLists.txts

cmake_minimum_required(VERSION 3.13.4)

set(LOGGO_EXAMPLE "loggo_example")

# Create examples
add_executable(${LOGGO_EXAMPLE} loggo_example.c)
target_include_directories(${LOGGO_EXAMPLE} PRIVATE ${CMAKE_SOURCE_DIR}/libs)
set_target_properties("${LOGGO_EXAMPLE}"
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


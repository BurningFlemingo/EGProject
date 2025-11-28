cmake_minimum_required(VERSION 3.8)

project(PEngine)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set (CMAKE_CXX_STANDARD 20)

add_subdirectory(PEngine)
add_subdirectory(examples)

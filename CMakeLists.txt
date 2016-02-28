cmake_minimum_required(VERSION 2.8)

project(renderer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include_directories(./src/)
include_directories(./src/util/)

add_subdirectory(src)
add_subdirectory(examples)
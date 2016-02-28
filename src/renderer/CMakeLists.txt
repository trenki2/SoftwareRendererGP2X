cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(renderer
	geometry_processor.cpp)
target_link_libraries(renderer)
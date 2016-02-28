cmake_minimum_required(VERSION 2.8)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

add_executable(example1 example1.cpp)
target_link_libraries(example1 renderer ${SDL_LIBRARY})

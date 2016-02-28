cmake_minimum_required(VERSION 2.8)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

add_executable(performance performance.cpp)
target_link_libraries(performance renderer ${SDL_LIBRARY})

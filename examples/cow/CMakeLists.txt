cmake_minimum_required(VERSION 2.8)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

add_executable(cow main.cpp)
target_link_libraries(cow renderer fixedpoint util ${SDL_LIBRARY})

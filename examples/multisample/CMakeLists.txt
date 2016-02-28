cmake_minimum_required(VERSION 2.8)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

add_executable(colorwheel_multisample colorwheel.cpp)
add_executable(triangle_multisample triangle.cpp)
add_executable(line_multisample line.cpp)
target_link_libraries(colorwheel_multisample renderer ${SDL_LIBRARY})
target_link_libraries(triangle_multisample renderer ${SDL_LIBRARY})
target_link_libraries(line_multisample renderer ${SDL_LIBRARY})

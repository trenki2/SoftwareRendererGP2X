cmake_minimum_required(VERSION 2.8)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

find_package(SDL_image REQUIRED)
include_directories(${SDLIMAGE_INCLUDE_DIR})

add_executable(example2 example2.cpp)
target_link_libraries(example2 renderer ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY})

cmake_minimum_required(VERSION 2.8)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

add_executable(cow_threaded1 cow_threaded1.cpp)
target_link_libraries(cow_threaded1 renderer fixedpoint util ${SDL_LIBRARY})

add_executable(cow_threaded2 cow_threaded2.cpp)
target_link_libraries(cow_threaded2 renderer fixedpoint util ${SDL_LIBRARY})

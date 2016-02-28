cmake_minimum_required(VERSION 2.8)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

if (WIN32)
    find_package(Pthread)
    include_directories(${PTHREAD_INCLUDE_DIR})
    set(THREAD_LIBS ${PTHREAD_LIBRARIES})
else (WIN32)
    # Assume Linux with pthread
    find_package(Threads)
    set(THREAD_LIBS ${CMAKE_THREAD_LIBS_INIT})
endif (WIN32)

add_executable(userdata userdata.cpp)
target_link_libraries(userdata renderer ${SDL_LIBRARY} ${THREAD_LIBS})
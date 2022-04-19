cmake_minimum_required(VERSION 3.21)
project(week07)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 90)

include_directories(.)

set(SOURCES threadpool.c)

add_executable(example.out example.cpp)

add_executable(a.out main.c)

add_executable(cyk.out cyk.c ${SOURCES})

target_link_libraries(cyk.out pthread)

add_executable(demo.out demo.c ${SOURCES})
target_link_libraries(demo.out pthread)

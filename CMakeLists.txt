cmake_minimum_required(VERSION 4.3)
project(melonframe C)

set(CMAKE_C_STANDARD 11)

add_executable(melonframe main.c
        melonframe.h
        melonframe.c
        tests/encoding.c
        tests/encoding.h
)

option(BUILD_SHARED_LIBS "Build melonframe as a shared library" OFF)

add_library(melonframe_lib
        melonframe.h
        melonframe.c
        tests/encoding.c
        tests/encoding.h
)
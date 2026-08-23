cmake_minimum_required(VERSION 4.3)
project(melonframe C)

set(CMAKE_C_STANDARD 11)

add_executable(melonframe main.c
        melonframe_crc.c
        melonframe_crc.h
        melonframe_encoder.c
        melonframe_encoder.h
        melonframe_decoder.c
        melonframe_decoder.h
        melonframe.h
)

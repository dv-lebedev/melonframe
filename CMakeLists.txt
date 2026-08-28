cmake_minimum_required(VERSION 4.3)
project(melonframe C)

set(CMAKE_C_STANDARD 11)

add_library(melonframe SHARED
        melonframe.c
)

target_include_directories(melonframe PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

enable_testing()

add_executable(test_encoding melonframe.h melonframe.c tests/test_encoding.c)
add_test(NAME TestEncoding COMMAND test_encoding)

add_executable(test_decoding melonframe.h melonframe.c tests/test_decoding.c)
add_test(NAME TestDecoding COMMAND test_decoding)

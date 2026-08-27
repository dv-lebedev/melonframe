cmake_minimum_required(VERSION 4.3)
project(melonframe C)

set(CMAKE_C_STANDARD 11)

include(GenerateExportHeader)

add_library(melonframe SHARED
        melonframe.c
)

generate_export_header(melonframe
        EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/melonframe_export.h"
)

target_include_directories(melonframe PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

if(WIN32)
    set_target_properties(melonframe PROPERTIES PREFIX "")
endif()

enable_testing()

add_executable(test_encoding melonframe.h melonframe.c tests/test_encoding.c)
add_test(NAME TestEncoding COMMAND test_encoding)

add_executable(test_decoding melonframe.h melonframe.c tests/test_decoding.c)
add_test(NAME TestDecoding COMMAND test_decoding)

cmake_minimum_required(VERSION 3.16)
project(wofl_audio_codec LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Core library (needs one .cpp at least)
add_library(woflcodec
    src/woflcodec.cpp
)

target_include_directories(woflcodec PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Encoder executable
add_executable(encoder src/encoder_main.cpp)
target_link_libraries(encoder PRIVATE woflcodec)

# Decoder executable
add_executable(decoder src/decoder_main.cpp)
target_link_libraries(decoder PRIVATE woflcodec)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

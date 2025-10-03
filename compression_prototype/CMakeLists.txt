cmake_minimum_required(VERSION 3.20)
project(compression_prototype CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# KISS FFT (C library)
add_library(kissfft STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/kissfft/kiss_fft.c
)
target_include_directories(kissfft PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/kissfft
)
target_compile_definitions(kissfft PUBLIC KISS_FFT_STATIC)

# WAV I/O
add_library(wav_io
    ${CMAKE_CURRENT_SOURCE_DIR}/wav_io.cpp
)
target_include_directories(wav_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Encoder / Decoder / Validate
add_executable(encoder ${CMAKE_CURRENT_SOURCE_DIR}/encoder.cpp)
target_link_libraries(encoder PRIVATE kissfft wav_io)

add_executable(decoder ${CMAKE_CURRENT_SOURCE_DIR}/decoder.cpp)
target_link_libraries(decoder PRIVATE kissfft wav_io)

add_executable(validate ${CMAKE_CURRENT_SOURCE_DIR}/validate.cpp)
target_link_libraries(validate PRIVATE wav_io)

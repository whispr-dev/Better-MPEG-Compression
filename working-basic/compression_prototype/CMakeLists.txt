
cmake_minimum_required(VERSION 3.16)
project(wofl_audio_codec LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(WOFL_BUILD_TESTS "Build tests" OFF)

add_library(woflcodec
    src/fft.hpp
    src/mdct.hpp
    src/bitio.hpp
    src/rice.hpp
    src/wav.hpp
    src/analysis.hpp
    src/psy.hpp
    src/bitstream.hpp
    src/parametric.hpp
    src/residual.hpp
    src/codebook.hpp
)
target_include_directories(woflcodec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(encoder src/encoder_main.cpp)
target_link_libraries(encoder PRIVATE woflcodec)

add_executable(decoder src/decoder_main.cpp)
target_link_libraries(decoder PRIVATE woflcodec)

if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

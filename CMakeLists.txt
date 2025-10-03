cmake_minimum_required(VERSION 3.10)
project(hi_speed_stream_processing_for_ai_mini_devices)

set(CMAKE_CXX_STANDARD 17)

# Only build the compression prototype (no external deps required)
add_subdirectory(compression_prototype)

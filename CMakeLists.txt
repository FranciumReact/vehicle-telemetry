cmake_minimum_required(VERSION 3.16)
project(vehicle_telemetry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Werror)

add_executable(test_decode
    tests/test_decode.cpp
    src/can_decode.cpp
    src/signal_table.cpp
    src/signal_encode.cpp
    src/frame_builder.cpp
)
target_include_directories(test_decode PRIVATE include)
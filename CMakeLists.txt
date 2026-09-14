cmake_minimum_required(VERSION 3.16)
project(vehicle_telemetry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Werror)

add_executable(encode_test
    src/signal_encode.cpp
    src/frame_builder.cpp
    src/vehicle_model.cpp
    src/main.cpp
    src/signal_table.cpp
    src/can_decode.cpp
    
)

target_include_directories(encode_test PRIVATE include)
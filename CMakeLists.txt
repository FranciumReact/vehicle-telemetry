cmake_minimum_required(VERSION 3.16)
project(vehicle_telemetry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

add_executable(encode_test
    src/signal_encode.cpp
    src/main.cpp
)

target_include_directories(encode_test PRIVATE include)
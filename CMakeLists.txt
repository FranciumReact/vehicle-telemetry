cmake_minimum_required(VERSION 3.16)
project(vehicle_telemetry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Werror)

find_package(Threads REQUIRED)

add_executable(encode_test
    src/main.cpp
    src/signal_encode.cpp
    src/frame_builder.cpp
    src/vehicle_model.cpp
    src/signal_table.cpp
    src/can_decode.cpp
    src/frame_queue.cpp
    src/validation.cpp
    src/rate_validator.cpp
    src/dtc_engine.cpp
)
target_include_directories(encode_test PRIVATE include)
target_link_libraries(encode_test PRIVATE Threads::Threads)

add_executable(test_decode
    tests/test_decode.cpp
    src/can_decode.cpp
    src/signal_table.cpp
    src/signal_encode.cpp
    src/frame_builder.cpp
    src/validation.cpp
    src/rate_validator.cpp
    src/dtc_engine.cpp
)
target_include_directories(test_decode PRIVATE include)
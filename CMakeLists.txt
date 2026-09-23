cmake_minimum_required(VERSION 3.16)
project(vehicle_telemetry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Werror)

find_package(Threads REQUIRED)

# ---------------------------------------------------------------- pipeline
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
    src/telemetry_writer.cpp
)
target_include_directories(encode_test PRIVATE include)
target_link_libraries(encode_test PRIVATE Threads::Threads pqxx pq)

# ---------------------------------------------------------------- tests
# FetchContent downloads and builds Catch2 at configure time, so nobody
# has to install it first and CI needs no extra steps.
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1
)
FetchContent_MakeAvailable(Catch2)

# The test binary links only what it exercises: no database, no threads.
add_executable(unit_tests
    tests/test_decode.cpp
    src/can_decode.cpp
    src/signal_table.cpp
    src/signal_encode.cpp
    src/frame_builder.cpp
    src/validation.cpp
    src/rate_validator.cpp
    src/dtc_engine.cpp
)
target_include_directories(unit_tests PRIVATE include)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)

enable_testing()
add_test(NAME unit_tests COMMAND unit_tests)
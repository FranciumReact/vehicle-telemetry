# CMake generated Testfile for 
# Source directory: /home/harsh/vehicle-telemetry
# Build directory: /home/harsh/vehicle-telemetry/build-asan
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(unit_tests "/home/harsh/vehicle-telemetry/build-asan/unit_tests")
set_tests_properties(unit_tests PROPERTIES  _BACKTRACE_TRIPLES "/home/harsh/vehicle-telemetry/CMakeLists.txt;54;add_test;/home/harsh/vehicle-telemetry/CMakeLists.txt;0;")
subdirs("_deps/catch2-build")

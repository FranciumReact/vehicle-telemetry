# Vehicle Telemetry and Diagnostic Platform

This is a simulated automotive telemetry and diagnostic system: fictional CAN protocal, C++ decoding and processing engine, PostgreSQL storage, and a web dashboard.

This project is not addiliated with, endorsed by, or derived from any vehicle manufacturer. Uses an invented protocol.

## Status

In development. Currently implemented:

- Fictional CAN protocol specification ([docs/can_protocol.md](docs/can_protocol.md))
- CAN signal encoding: 8-bit, 16-bit little-endian, and arbitrary bit fields
- Frame builders for all five message types
- A simple vehicle model producing a plausible drive cycle

## Building

```bash
mkdir -p build && cd build
cmake ..
make
./encode_test
```

Requires CMake 3.16+ and a C++17 compiler.

## What it does today

Simulates ten seconds of driving and prints the resulting CAN frames as a
hexdump — four ECUs broadcasting engine, dynamics, battery, and powertrain
data derived from an evolving vehicle state.
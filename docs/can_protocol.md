# Ficitional Control Area Network(CAN) Protocol

## Overwiew

Fictional CAN protocol used by the Vehicle Telemetry and Diagnostic Platform

The protocol uses classical CAN with 11-bit identifiers and payloads of up to 8 bytes. All the multi-byte signals are little endians, where the LSB is first. All signals are unsigned unless stated otherwise.

# Message Summary

| CAN ID | Name               | Sender | DLC | Cycle Time      |
|--------|--------------------|--------|-----|-----------------|
| 0x100  | ENGINE_DATA        | ECM    | 8   | 10 ms           |
| 0x101  | VEHICLE_DYNAMICS   | ABS    | 8   | 10 ms           |
| 0x102  | BATTERY_DATA       | BCM    | 8   | 1000 ms         |
| 0x103  | POWERTRAIN_STATUS  | TCM    | 8   | 100 ms          |
| 0x300  | DIAGNOSTIC_DATA    | any    | 8   | event + 1000 ms |

## Decode Rule

All the scaled signals are deocoded using the formula:
    Physical = raw x factor + offset

Enumerated and booleans signals are use factor 1 and offset 0, their raw values map directly to a defined state.
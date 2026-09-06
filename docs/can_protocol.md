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

## Messages

### 0x100 ENGINE_DATA

Sender: Engine COntrol Module (ECM). DLC 8. Cycle 10ms.

| Signal      | Start | Len | Factor | Offset | Unit | Range     |
|-------------|-------|-----|--------|--------|------|-----------|
| EngineRPM   | 0     | 16  | 0.25   | 0      | rpm  | 0–8000    |
| ThrottlePos | 16    | 8   | 0.4    | 0      | %    | 0–100     |
| EngineLoad  | 24    | 8   | 0.4    | 0      | %    | 0–100     |
| CoolantTemp | 32    | 8   | 1      | −40    | °C   | −40–215   |
| reserved    | 40    | 24  | —      | —      | —    | —         |

### 0x101 VEHICLE_DYNAMICS

Sender: Anti-lock Braking System module (ABS). DLC 8. Cycle 10 ms.

| Signal         | Start | Len | Factor | Offset | Unit | Range   |
|----------------|-------|-----|--------|--------|------|---------|
| VehicleSpeed   | 0     | 16  | 0.01   | 0      | km/h | 0–300   |
| AcceleratorPos | 16    | 8   | 0.4    | 0      | %    | 0–100   |
| BrakePressed   | 24    | 1   | 1      | 0      | bool | 0–1     |
| reserved       | 25    | 39  | —      | —      | —    | —       |

### 0x102 BATTERY_DATA

Sender: Body Control Module (BCM). DLC 8. Cycle 1000 ms.

| Signal         | Start | Len | Factor | Offset | Unit | Range   |
|----------------|-------|-----|--------|--------|------|---------|
| BatteryVoltage | 0     | 16  | 0.01   | 0      | V    | 0–20    |
| AmbientTemp    | 16    | 8   | 1      | −40    | °C   | −40–80  |
| reserved       | 24    | 40  | —      | —      | —    | —       |

### 0x103 POWERTRAIN_STATUS

Sender: Transmission Control Module (TCM). DLC 8. Cycle 100 ms.

| Signal            | Start | Len | Factor | Offset | Unit | Range |
|-------------------|-------|-----|--------|--------|------|-------|
| TransmissionState | 0     | 3   | 1      | 0      | enum | 0–4   |
| FuelRate          | 3     | 16  | 0.1    | 0      | L/h  | 0–60  |
| reserved          | 19    | 45  | —      | —      | —    | —     |

Note: FuelRate begins at bit 3 and therefore crosses byte boubndaries. Decoders must handle non byte aligned signals.

### 0x300 DIAGNOSTIC_DATA

Sender: any ECU. DLC 8. Event-driven, repeated every 1000 ms while a fault
remains active so that late-joining receivers observe it.

| Signal      | Start | Len | Factor | Offset | Unit | Range     |
|-------------|-------|-----|--------|--------|------|-----------|
| DtcCode     | 0     | 16  | 1      | 0      | enum | 0–65535   |
| DtcSeverity | 16    | 2   | 1      | 0      | enum | 0–3       |
| DtcActive   | 18    | 1   | 1      | 0      | bool | 0–1       |
| SourceEcu   | 19    | 4   | 1      | 0      | enum | 0–15      |
| reserved    | 23    | 41  | —      | —      | —    | —         |
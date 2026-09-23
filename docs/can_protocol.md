# Fictional Controller Area Network (CAN) Protocol

## Overview

A fictional CAN protocol used by the Vehicle Telemetry and Diagnostics
Platform. It is invented for this project and is not derived from,
compatible with, or based on any manufacturer's proprietary protocol.

The protocol uses classic CAN with 11-bit identifiers and payloads of up to
8 bytes. All multi-byte signals are little-endian, least significant byte
first. All signals are unsigned unless stated otherwise.

## Message summary

| CAN ID | Name               | Sender | DLC | Cycle Time      |
|--------|--------------------|--------|-----|-----------------|
| 0x100  | ENGINE_DATA        | ECM    | 8   | 10 ms           |
| 0x101  | VEHICLE_DYNAMICS   | ABS    | 8   | 10 ms           |
| 0x102  | BATTERY_DATA       | BCM    | 8   | 1000 ms         |
| 0x103  | POWERTRAIN_STATUS  | TCM    | 8   | 100 ms          |
| 0x300  | DIAGNOSTIC_DATA    | any    | 8   | event + 1000 ms |

## Decode rule

All scaled signals are decoded with:

    physical = raw × factor + offset

Enumerated and boolean signals use factor 1 and offset 0; their raw values
map directly to a defined state.

## Signal attributes

Each signal below specifies:

- **Start / Len** — bit position and width within the 8-byte payload.
- **Factor / Offset** — the decode rule above.
- **Range** — the physically plausible range, used for validation. This is
  deliberately *not* the range the encoding can represent. CoolantTemp, for
  example, can encode up to 215 °C, but its valid range stops at 130 °C:
  a higher reading means a broken sensor, not a hot engine. Using the
  encoding limit would mean validation only ever caught encoding errors.
- **Max rate** — the largest plausible change per second, used for
  rate-of-change validation.

## Messages

### 0x100 ENGINE_DATA

Sender: Engine Control Module (ECM). DLC 8. Cycle 10 ms.

| Signal      | Start | Len | Factor | Offset | Unit | Range     | Max rate |
|-------------|-------|-----|--------|--------|------|-----------|----------|
| EngineRPM   | 0     | 16  | 0.25   | 0      | rpm  | 0–8000    | 3000/s   |
| ThrottlePos | 16    | 8   | 0.4    | 0      | %    | 0–100     | 200/s    |
| EngineLoad  | 24    | 8   | 0.4    | 0      | %    | 0–100     | 200/s    |
| CoolantTemp | 32    | 8   | 1      | −40    | °C   | −40–130   | 150/s    |
| reserved    | 40    | 24  | —      | —      | —    | —         | —        |

### 0x101 VEHICLE_DYNAMICS

Sender: Anti-lock Braking System module (ABS). DLC 8. Cycle 10 ms.

| Signal         | Start | Len | Factor | Offset | Unit | Range   | Max rate |
|----------------|-------|-----|--------|--------|------|---------|----------|
| VehicleSpeed   | 0     | 16  | 0.01   | 0      | km/h | 0–300   | 30/s     |
| AcceleratorPos | 16    | 8   | 0.4    | 0      | %    | 0–100   | 200/s    |
| BrakePressed   | 24    | 1   | 1      | 0      | bool | 0–1     | unbounded|
| reserved       | 25    | 39  | —      | —      | —    | —       | —        |

### 0x102 BATTERY_DATA

Sender: Body Control Module (BCM). DLC 8. Cycle 1000 ms.

| Signal         | Start | Len | Factor | Offset | Unit | Range   | Max rate |
|----------------|-------|-----|--------|--------|------|---------|----------|
| BatteryVoltage | 0     | 16  | 0.01   | 0      | V    | 0–20    | 5/s      |
| AmbientTemp    | 16    | 8   | 1      | −40    | °C   | −40–80  | 150/s    |
| reserved       | 24    | 40  | —      | —      | —    | —       | —        |

### 0x103 POWERTRAIN_STATUS

Sender: Transmission Control Module (TCM). DLC 8. Cycle 100 ms.

| Signal            | Start | Len | Factor | Offset | Unit | Range | Max rate  |
|-------------------|-------|-----|--------|--------|------|-------|-----------|
| TransmissionState | 0     | 3   | 1      | 0      | enum | 0–4   | unbounded |
| FuelRate          | 3     | 16  | 0.1    | 0      | L/h  | 0–60  | 30/s      |
| reserved          | 19    | 45  | —      | —      | —    | —     | —         |

Note: FuelRate begins at bit 3 and therefore crosses byte boundaries.
Decoders must handle non-byte-aligned signals.

### 0x300 DIAGNOSTIC_DATA

Sender: any ECU. DLC 8. Event-driven, repeated every 1000 ms while a fault
remains active so that late-joining receivers observe it.

| Signal      | Start | Len | Factor | Offset | Unit | Range   | Max rate  |
|-------------|-------|-----|--------|--------|------|---------|-----------|
| DtcCode     | 0     | 16  | 1      | 0      | enum | 0–65535 | unbounded |
| DtcSeverity | 16    | 2   | 1      | 0      | enum | 0–3     | unbounded |
| DtcActive   | 18    | 1   | 1      | 0      | bool | 0–1     | unbounded |
| SourceEcu   | 19    | 4   | 1      | 0      | enum | 0–15    | unbounded |
| reserved    | 23    | 41  | —      | —      | —    | —       | —         |

## Enumerations

### TransmissionState

| Value | State              |
|-------|--------------------|
| 0     | Park               |
| 1     | Reverse            |
| 2     | Neutral            |
| 3     | Drive              |
| 4     | Sport              |
| 5–7   | Reserved / invalid |

### DtcSeverity

| Value | Severity |
|-------|----------|
| 0     | Info     |
| 1     | Warning  |
| 2     | Severe   |
| 3     | Critical |

### SourceEcu

| Value | ECU      |
|-------|----------|
| 0     | ECM      |
| 1     | TCM      |
| 2     | BCM      |
| 3     | ABS      |
| 4–15  | Reserved |

## Diagnostic trouble codes

All codes are fictional and defined solely for this project.

| Code  | Description                            | Severity |
|-------|----------------------------------------|----------|
| P0001 | Coolant temperature above limit        | Severe   |
| P0002 | Battery voltage below limit            | Warning  |
| P0003 | Engine RPM above limit                 | Critical |
| P0004 | Sensor value outside physical range    | Warning  |
| P0005 | Implausible rate of change             | Warning  |

## Design decisions

### Identifier assignment

Lower identifiers are higher priority and are handled first. This works
because arbitration is bit by bit: a dominant 0 overwrites a recessive 1, so
the numerically smaller identifier survives and the losing node backs off
without corrupting the frame in progress. 0x300 for diagnostics is
acceptable because a fault report arriving 20 ms late is harmless, whereas a
delayed brake message is not.

### Cycle time selection

Cycle time tracks how fast the physical quantity actually changes. Engine
RPM can go from idle to redline in under a second, so a slow cycle would
miss the shape of the curve entirely. Ambient temperature moves over
minutes, so 1000 ms is ample. Over-sampling a slow signal wastes bus
bandwidth on redundant frames, and bandwidth is finite and shared.

### Brake signal consolidation

A dedicated brake message was considered and rejected. BrakePressed comes
from the same ECU (ABS) on the same 10 ms cycle as vehicle speed, and
signals that share a sender and a cycle belong in the same frame. Spending a
whole 8-byte frame on a single bit would waste bandwidth. 0x200 remains
reserved for future brake pressure and per-wheel speeds, which would justify
their own frame.

### Reserved bits

Every message is 8 bytes even where its signals use fewer. Adding a signal
later therefore does not change the DLC, so existing decoders keep working
unchanged — which matters when ECUs from different suppliers ship on
different schedules and cannot all be updated at once. Fixed-size frames
also simplify buffer handling in the decoder. Reserved bits transmit as zero
and receivers must ignore them rather than assume a value.

### Rate limits on quantised signals

Rate-of-change validation is bounded by a signal's own resolution. The
temperature signals use a factor of 1, so the smallest change they can
represent is 1 °C. At a 10 ms cycle a single quantisation step therefore
registers as 100 °C/sec, and any rate limit below that would fire on every
step of a perfectly normal warm-up. Their limits are set above that floor
accordingly.

Enumerated and boolean signals are marked unbounded because a legitimate
transition — Park to Drive, brake off to on — is instantaneous by nature and
carries no rate information.
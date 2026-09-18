# Vehicle Telemetry & Diagnostics Platform

A simulated automotive telemetry system: a fictional CAN protocol, a C++
encoder/decoder, and a threaded processing pipeline with validation and
diagnostics. Built as a learning project to understand how vehicle networks
and telemetry software work.

**This is an educational project.** It is not affiliated with, endorsed by,
or derived from any vehicle manufacturer. The CAN protocol is invented for
this project. No real vehicle data or proprietary specification is used.

## Motivation

Modern vehicles run dozens of electronic control units communicating over
CAN, a broadcast bus developed by Bosch in 1986. I wanted to understand how
sensor values get packed into 8-byte frames, how they're recovered, and how
a telemetry system processes that stream without dropping data.

Rather than read about it, I built a simplified version end to end.

## Current status

Milestones 1-6 of 13 complete.

| Component | Status |
|---|---|
| Fictional CAN protocol specification | Done |
| Signal encoding (8/16-bit, bit fields) | Done |
| Vehicle model and frame generation | Done |
| Table-driven decoder | Done |
| Threaded processing pipeline | Done |
| Signal validation (range and rate) | Done |
| Diagnostic trouble code engine | Done |
| Database storage | Not started |
| Web dashboard | Not started |
| Python analytics | Not started |

## Architecture

```
Vehicle model -> Frame builders -> Bounded queue -> Decoder -> Validation -> DTC engine
  (physics)       (bit packing)    (thread-safe)  (table-driven)  (range/rate)  (debounced)
```

The producer thread simulates the vehicle and emits CAN frames at the cycle
times defined in the protocol spec. The consumer thread decodes, validates,
and runs diagnostics on them. A bounded queue with a mutex and condition
variable connects the two, so a slow consumer never stalls the producer.

## The CAN protocol

Five messages, fully documented in [docs/can_protocol.md](docs/can_protocol.md).

| CAN ID | Name | Sender | Cycle |
|---|---|---|---|
| 0x100 | ENGINE_DATA | ECM | 10 ms |
| 0x101 | VEHICLE_DYNAMICS | ABS | 10 ms |
| 0x102 | BATTERY_DATA | BCM | 1000 ms |
| 0x103 | POWERTRAIN_STATUS | TCM | 100 ms |
| 0x300 | DIAGNOSTIC_DATA | any | event-driven |

Every signal specifies a start bit, length, scaling factor, offset, unit,
valid range, and maximum plausible rate of change. The decoder and both
validators are driven entirely by this table — adding a signal requires no
code changes.

Signals are not all byte-aligned. FuelRate, for example, is 16 bits
starting at bit 3, so it straddles three bytes.

## Validation and diagnostics

These are deliberately separate concerns.

**Validation** asks whether a reading is physically possible. A coolant
temperature of 200 °C is not a hot engine, it's a broken sensor. Range
checks compare against per-signal bounds; rate checks compare against the
previous reading and reject changes faster than the real world allows.

**Diagnostics** ask whether the vehicle is behaving badly. A coolant
temperature of 115 °C is a perfectly valid reading that indicates
overheating. The data is trustworthy; the vehicle isn't.

Diagnostic trouble codes are debounced: a condition must hold for 10
consecutive cycles before a code is confirmed, and clear for 10 cycles
before it heals. This mirrors how production ECUs avoid setting codes on a
single noisy sample. Only state transitions are reported as events, not the
ongoing condition.

The difference shows in the output. A 15-frame injected overheat produces 15
range violations, 2 rate violations (the jump in and the recovery), and
exactly one confirmed DTC:

```
DTC P0001 SET at t=2.10
DTC P0001 CLEARED at t=2.25
decoded 500, out of range 15, implausible rate 2, dropped 0
```

Isolated single-frame spikes produce range violations but no DTC at all,
which is the debouncing working as intended.

## Design decisions

**Table-driven decoding.** Signal parameters live in one data structure
rather than being hardcoded at each call site. This came from getting
burned: a mistyped scaling factor (0.04 instead of 0.4) produced
plausible-looking but wrong output with no error.

**Reject malformed frames rather than partially decode them.** A frame
whose DLC doesn't match the spec is discarded and counted. Partial decoding
would mean bounds-checking every signal individually, and a sensor value
you can't trust is worse than no value.

**Bounded queue with a drop counter.** An unbounded queue under sustained
overload ends in an out-of-memory kill. Dropping the oldest frame and
counting the loss makes overload visible and survivable.

**std::optional for fallible operations.** Decoding can fail on an unknown
ID or a DLC mismatch. Returning an optional makes the failure explicit
rather than smuggling it through a sentinel value.

**Validation bounds are physical, not representational.** CoolantTemp can
encode up to 215 °C, but its valid range stops at 130 °C. Using the encoding
limit would mean validation only ever catches encoding errors, never sensor
faults.

## Performance

Measured on a 13th Gen Intel Core i9-13900H (WSL2, Ubuntu), single producer
and single consumer thread:

- Decode throughput: ~645,000 frames/sec unthrottled
  (three runs: 650k / 642k / 647k)
- Zero frames dropped when the producer emits at protocol cycle times

Under the unthrottled test the producer outruns the consumer and roughly 90%
of frames are evicted by the queue's drop policy. That is the intended
behaviour under overload — bounded memory, counted loss — not a failure.

For context: a classic CAN frame with 8 data bytes occupies roughly 110-130
bits on the wire once identifier, CRC, ACK, and stuffing overhead are
included. At 500 kbit/s that puts the theoretical ceiling near 4,000 frames
per second, and real buses are typically run well below saturation because
arbitration latency degrades as load approaches the limit.

## Building

Requires CMake 3.16+ and a C++17 compiler.

```bash
mkdir -p build && cd build
cmake ..
make
./encode_test    # runs a 5-second drive cycle with an injected fault
./test_decode    # runs the decoder tests
```

Built with `-Wall -Wextra -Werror`.

## Testing

Round-trip tests encode known physical values, decode them back, and assert
the result is within one quantisation step. Exact float comparison is not
valid here — 33% throttle encodes to a raw value that decodes as 33.2%,
because an 8-bit field with factor 0.4 has 0.4% resolution.

Rejection tests confirm that unknown identifiers and DLC mismatches are
refused rather than silently decoded.

Fault injection in the simulator exercises both validators and the DTC
engine on every run.

## Limitations

- Simulated CAN only. No hardware, no SocketCAN, no real vehicle data.
- The vehicle model is crude and not physically accurate. It produces
  plausible-looking values, not a validated engine simulation.
- No AUTOSAR, ISO 26262, or MISRA compliance. This is not safety-relevant
  code and makes no claim to be.
- Classic CAN only: 11-bit identifiers, 8-byte payloads. No CAN FD.
- Little-endian signals only, though the design allows byte order to be a
  per-signal property.
- Rate-of-change validation is bounded by signal quantisation. A coarse
  signal such as coolant temperature (1 °C steps at a 10 ms cycle) registers
  a single step as 100 °C/sec, so meaningful rate limits below that are not
  enforceable at this sample rate.
- The DTC engine detects threshold conditions in telemetry. It does not
  diagnose mechanical faults and makes no predictive claims.

## Planned

PostgreSQL storage for sessions and telemetry history, a web dashboard,
Python-based statistical anomaly detection, containerisation, and CI.

## Repository layout

```
include/     headers
src/         implementation
tests/       unit tests
docs/        protocol specification
```
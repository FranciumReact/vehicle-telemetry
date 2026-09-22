#include "signal_table.hpp"

// 0x100 Spec Table
// NOTE: temperature signals use a 1 C quantisation step. At a 10 ms sample
// rate a single step registers as 100 C/sec, so any rate limit below that
// fires on every step. Their max_rate is set above that floor accordingly.
const std::vector<MessageSpec> &get_message_table() {
  static const std::vector<MessageSpec> table = {
      {0x100,
       "ENGINE_DATA",
       8,
       {
           {"EngineRPM", 0, 16, 0.5, 0.0, "rpm", 0.0, 8000.0,3000.0},
           {"ThrottlePos", 16, 8, 0.4, 0.0, "%", 0.0, 100.0, 200.0},
           {"EngineLoad", 24, 8, 0.4, 0.0, "%", 0.0, 100.0, 200.0},
           {"CoolantTemp", 32, 8, 1.0, -40.0, "C", -40.0, 130.0, 150.0},
       }},
      {0x101,
       "VEHICLE_DYNAMICS",
       8,
       {
           {"VehicleSpeed", 0, 16, 0.01, 0.0, "km/h", 0.0, 300.0, 30.0},
           {"AcceleratorPos", 16, 8, 0.4, 0.0, "%", 0.0, 100.0, 200.0},
           {"BrakePressed", 24, 1, 1.0, 0.0, "bool", 0.0, 1.0, 1e9},
       }},
      {0x102,
       "BATTERY_DATA",
       8,
       {
           {"BatteryVoltage", 0, 16, 0.01, 0.0, "V", 0.0, 20.0, 5.0},
           {"AmbientTemp", 16, 8, 1.0, -40.0, "C", -40.0, 80.0, 1.0},
       }},
      {0x103,
       "POWERTRAIN_STATUS",
       8,
       {
           {"TransmissionState", 0, 3, 1.0, 0.0, "enum", 0.0, 4.0, 1e9},
           {"FuelRate", 3, 16, 0.1, 0.0, "L/h", 0.0, 60.0, 30.0},

       }},
      {0x300,
       "DIAGNOSTIC_DATA",
       8,
       {
           {"DtcCode", 0, 16, 1.0, 0.0, "enum", 0.0, 65535.0, 1e9},
           {"DtcSeverity", 16, 2, 1.0, 0.0, "enum", 0.0, 3.0, 1e9},
           {"DtcActive", 18, 1, 1.0, 0.0, "bool", 0.0, 1.0, 1e9},
           {"SourceEcu", 19, 4, 1.0, 0.0, "enum", 0.0, 15.0, 1e9},
       }},
  };
  return table;
}
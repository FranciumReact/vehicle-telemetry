#pragma once
#include <cstdint>

struct VehicleState {
    double speed_kmh    = 0.0;
    double rpm          = 800.0;   // idle
    double throttle_pct = 0.0;
    double load_pct     = 0.0;
    double coolant_c    = 20.0;    // ambient at start
    double voltage_v    = 12.6;
    double ambient_c    = 20.0;
    double fuel_lph     = 0.8;
    uint8_t gear        = 0;       // Park
    bool brake          = false;
};

void update_vehicle(VehicleState& s, double dt_s);
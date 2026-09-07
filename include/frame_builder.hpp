#pragma once
#include "can_frame.hpp"

CanFrame build_engine_data(double rpm, double throttle, double load, double coolant);
CanFrame build_vehicle_dynamics(double speed, double accel, bool brake);
CanFrame build_battery_data(double voltage, double ambient);
CanFrame build_powertrain_status(uint8_t gear, double fuel_rate);
CanFrame build_diagnostic_data(uint16_t code, uint8_t severity, bool active, uint8_t source_ecu);
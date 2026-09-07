#pragma once
#include "can_frame.hpp"

CanFrame build_engine_data(double rpm, double throttle, double load, double coolant);
CanFrame build_vehicle_dynamics(double speed, double accel, bool brake);
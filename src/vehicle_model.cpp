#include "vehicle_model.hpp"
#include <cmath>

void update_vehicle(VehicleState& s, double dt_s) {
    static double t = 0.0;
    t += dt_s;

    // Throttle: slow sinusoidal drive cycle, 0-80%
    s.throttle_pct = 40.0 + 40.0 * std::sin(t * 0.1);

    // Brake when throttle is low
    s.brake = (s.throttle_pct < 10.0);

    // Speed: throttle accelerates, drag and braking decelerate
    double accel = s.throttle_pct * 0.05;
    double drag  = s.speed_kmh * 0.02;
    double brake_force = s.brake ? 2.0 : 0.0;
    s.speed_kmh += (accel - drag - brake_force) * dt_s;
    if (s.speed_kmh < 0.0) s.speed_kmh = 0.0;

    // RPM tracks speed with an idle floor
    s.rpm = 800.0 + s.speed_kmh * 35.0;

    // Load tracks throttle
    s.load_pct = s.throttle_pct * 0.9;

    // Coolant approaches 90 C asymptotically (exponential warm-up)
    s.coolant_c += (90.0 - s.coolant_c) * 0.01 * dt_s;

    // Voltage: alternator charging, small ripple
    s.voltage_v = 13.8 + 0.1 * std::sin(t * 0.5);

    // Fuel rate scales with load
    s.fuel_lph = 0.8 + s.load_pct * 0.15;

    // Gear: Park at rest, Drive when moving
    s.gear = (s.speed_kmh > 1.0) ? 3 : 0;
}
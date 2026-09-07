#include "frame_builder.hpp"
#include "vehicle_model.hpp"
#include <cstdio>

int main() {
    // Simulates 10 seconds of a vehicle driving, one tick per second.
    // Every tick advances the physics, then packs the resulting state into
    // CAN frames the same way real ECUs would broadcast it.
    VehicleState s;   // starts stopped: 0 km/h, 800 rpm idle, 20 C coolant

    for (int i = 0; i < 10; i++) {
        // Advance the model by 1 second. Passed by reference, so this
        // mutates s directly instead of working on a copy.
        update_vehicle(s, 1.0);

        // Each builder is a different ECU broadcasting what it owns.
        // Physical units go in, packed 8-byte payloads come out.
        CanFrame ef = build_engine_data(s.rpm, s.throttle_pct, s.load_pct, s.coolant_c);
        CanFrame vd = build_vehicle_dynamics(s.speed_kmh, s.throttle_pct, s.brake);
        CanFrame bd = build_battery_data(s.voltage_v, s.ambient_c);
        CanFrame ps = build_powertrain_status(s.gear, s.fuel_lph);

        // Print them as a hexdump so we can read the bus by eye.
        // In a real system these would go out over the wire, not to stdout.
        CanFrame frames[4] = {ef, vd, bd, ps};
        for (const CanFrame& f : frames) {
            printf("[%2d] 0x%03X ", i, f.id);
            for (int b = 0; b < f.dlc; b++) printf("%02X ", f.data[b]);
            printf("\n");
        }
    }

    return 0;
}
#include "frame_builder.hpp"
#include "signal_encode.hpp"

CanFrame build_engine_data(double rpm, double throttle, double load, double coolant){

    CanFrame f{}; // Initialize everything to zero

    // Casts to a raw ID value
    f.id = static_cast<uint32_t>(CanId::EngineData);
    f.dlc = 8;

    // (physical, factor, offset) and bytes 5-7 are reserved
    encode_u16_le(f.data, 0, rpm, 0.25, 0.0); // Engine RPM which is bits 0-15
    f.data[2] = encode_u8(throttle, 0.4, 0.0); // ThrottlePos, bits 16-23
    f.data[3] = encode_u8(load, 0.4, 0.0); // EngineLoad, bits 24-31
    f.data[4] = encode_u8(coolant, 1.0, -40.0); // CoolantTemp, bits 32-39

    return f;
}

CanFrame build_vehicle_dynamics(double speed, double accel, bool brake){

    CanFrame f{};


    f.id = static_cast<uint32_t>(CanId::VehicleDynamics);
    f.dlc = 8;
    encode_u16_le(f.data, 0, speed, 0.01, 0.0); // VehicleSpeed which is bits 0-15
    f.data[2] = encode_u8(accel, 0.4, 0.0); // AcceleratorPos, bits 16-23
    set_bit(f.data, 24, brake);

    return f;
   

}
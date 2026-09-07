// Stops headers from being included in more than once
#pragma once

#include <cstdint>

// Create named set of CAN message IDs stored as 32-bit unsigned values, where each number is scoped
// and wont implicitly convert to a number
enum class CanId : uint32_t {
    EngineData = 0x100,
    VehicleDynamics = 0x101,
    BatteryData = 0x102,
    PowertrainStatus = 0x103,
    DiagnosticData = 0x300,
};
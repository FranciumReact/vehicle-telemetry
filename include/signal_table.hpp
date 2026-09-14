#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct SignalSpec {
    std::string name; // EngineRPM
    int         start_bit;
    int         length;
    double      factor;
    double      offset;
    std::string unit;
    double      min_value;
    double      max_value;
};

struct MessageSpec {
    uint32_t                id;
    std::string             name;
    uint8_t                 dlc;
    std::vector<SignalSpec> signals;
};

const std::vector<MessageSpec>& get_message_table();
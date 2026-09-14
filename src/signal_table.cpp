#include "signal_table.hpp"

// 0x100 Spec Table
const std::vector<MessageSpec>& get_message_table() {
    static const std::vector<MessageSpec> table = {
        {
            0x100, "ENGINE_DATA", 8,
            {
                {"EngineRPM",   0,  16, 0.25, 0.0,   "rpm", 0.0,   8000.0},
                {"ThrottlePos", 16, 8,  0.4,  0.0,   "%",   0.0,   100.0},
                {"EngineLoad",  24, 8,  0.4,  0.0,   "%",   0.0,   100.0},
                {"CoolantTemp", 32, 8,  1.0,  -40.0, "C",   -40.0, 215.0},
            }
        },
    };
    return table;
}
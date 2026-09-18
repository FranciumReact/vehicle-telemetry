#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include "can_decode.hpp"

enum class Severity : uint8_t { Info = 0, Warning = 1, Severe = 2, Critical = 3 };

struct DtcEvent {
    std::string code;
    Severity    severity;
    bool        active;
    double      time_s;
};

class DtcEngine {
public:
    // Feeds one decoded frame in; returns any DTCs that changed state.
    std::vector<DtcEvent> update(const DecodedFrame& values, double time_s);

private:
    void evaluate(const std::string& code, Severity sev, bool condition,
                  double time_s, std::vector<DtcEvent>& out);

    struct DtcState {
        int  fail_count = 0;
        int  heal_count = 0;
        bool active     = false;
    };
    std::unordered_map<std::string, DtcState> states_;

    static constexpr int kFailThreshold = 10;
    static constexpr int kHealThreshold = 10;
};
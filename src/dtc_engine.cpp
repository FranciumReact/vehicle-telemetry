#include "dtc_engine.hpp"

void DtcEngine::evaluate(const std::string& code, Severity sev, bool condition,
                         double time_s, std::vector<DtcEvent>& out) {
    DtcState& st = states_[code];   // creates a default entry if absent

    if (condition) {
        st.fail_count++;
        st.heal_count = 0;
    } else {
        st.heal_count++;
        st.fail_count = 0;
    }

    // your transition logic here
    if (!st.active && st.fail_count >= kFailThreshold){
        st.active = true;
        out.push_back({code, sev, true, time_s});
    } else if (st.active && st.heal_count >= kHealThreshold) {
        st.active = false;
        out.push_back({code, sev, false, time_s});
    }
}

std::vector<DtcEvent> DtcEngine::update(const DecodedFrame& values, double time_s) {
    std::vector<DtcEvent> events;

    auto it = values.find("CoolantTemp");
    if (it != values.end())
        evaluate("P0001", Severity::Severe, it->second > 110.0, time_s, events);

    it = values.find("BatteryVoltage");
    if (it != values.end())
        evaluate("P0002", Severity::Warning, it->second < 11.5, time_s, events);

    it = values.find("EngineRPM");
    if (it != values.end())
        evaluate("P0003", Severity::Critical, it->second > 7000.0, time_s, events);

    return events;
}
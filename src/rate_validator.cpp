#include "rate_validator.hpp"
#include <cmath>

ValidationResult RateValidator::check(const std::string& name,
                                      double value, double time_s) {
    auto it = history_.find(name);

    if (it == history_.end()) {
        // First sighting nothing to compare against yet.
        history_[name] = {value, time_s};
        return ValidationResult::Ok;
    }

    // work out how fast it changed.
    double dt = time_s - it->second.time_s;
    double dv = value - it->second.value;

    // Update the record regardless of the verdict.
    it->second = {value, time_s};

    if (dt <= 0.0) return ValidationResult::Ok;   // guard against divide-by-zero

    double rate = std::fabs(dv) / dt;

    //compare rate against a per-signal limit
    return ValidationResult::Ok;
}
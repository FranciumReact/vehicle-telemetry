#include "rate_validator.hpp"
#include <cmath>

ValidationResult RateValidator::check(const SignalSpec& spec,
                                      double value, double time_s) {
    auto it = history_.find(spec.name);

    if (it == history_.end()) {
        // First sighting, nothing to compare against yet.
        history_[spec.name] = {value, time_s};
        return ValidationResult::Ok;
    }

    // Work out how fast it changed.
    double dt = time_s - it->second.time_s;
    double dv = value - it->second.value;

    // Update the record regardless of the verdict.
    it->second = {value, time_s};

    if (dt <= 0.0) return ValidationResult::Ok;   // guard against divide-by-zero

    double rate = std::fabs(dv) / dt;

    // Compare against this signal's plausible rate of change.
    // The limit comes from the table, not from hardcoded logic here.
    if (rate > spec.max_rate) {
        return ValidationResult::ImplausibleRate;
    }

    return ValidationResult::Ok;
}
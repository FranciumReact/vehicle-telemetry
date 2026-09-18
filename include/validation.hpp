#pragma once
#include "signal_table.hpp"

enum class ValidationResult {
    Ok,
    OutOfRange,
    ImplausibleRate
};

ValidationResult validate_signal(const SignalSpec& spec, double value);
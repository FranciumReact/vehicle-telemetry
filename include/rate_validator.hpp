#pragma once
#include <string>
#include <unordered_map>
#include "validation.hpp"
#include "signal_table.hpp"

class RateValidator {
public:
    ValidationResult check(const SignalSpec& spec, double value, double time_s);

private:
    struct Previous {
        double value;
        double time_s;
    };
    std::unordered_map<std::string, Previous> history_;
};
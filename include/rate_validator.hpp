#pragma once
#include <string>
#include <unordered_map>
#include "validation.hpp"

class RateValidator {
public:
    ValidationResult check(const std::string& name, double value, double time_s);

private:
    struct Previous {
        double value;
        double time_s;
    };
    std::unordered_map<std::string, Previous> history_;
};
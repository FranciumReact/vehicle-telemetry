#include "validation.hpp"

ValidationResult validate_signal(const SignalSpec& spec, double value){
    if (value < spec.min_value || value > spec.max_value){
        return ValidationResult::OutOfRange;
    }
    return ValidationResult::Ok;
}
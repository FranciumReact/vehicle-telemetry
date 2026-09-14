#pragma once
#include <cstdint>
#include "signal_table.hpp"


uint32_t extract_bits(const uint8_t* data, int start_bit, int length);
double decode_signal(const SignalSpec& spec, const uint8_t* data);
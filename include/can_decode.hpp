#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <string>
#include "signal_table.hpp"
#include "can_frame.hpp"

uint32_t extract_bits(const uint8_t* data, int start_bit, int length);
double decode_signal(const SignalSpec& spec, const uint8_t* data);

using DecodedFrame = std::unordered_map<std::string, double>;
// hashmap signal name to the physical value
std::optional<DecodedFrame> decode_frame(const CanFrame& frame);
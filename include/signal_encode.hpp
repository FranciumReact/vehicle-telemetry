// Header file for the functions

#pragma once
#include <cstdint>
uint8_t encode_u8(double physical, double factor, double offset);
void encode_u16_le(uint8_t* data, int start_byte, double physical, double factor, double offset);
void set_bit(uint8_t* data, int bit_pos, bool value);
void set_bits(uint8_t* data, int start_bit, int length, uint32_t value);

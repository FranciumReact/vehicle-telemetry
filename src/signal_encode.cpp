#include <cstdint>
#include <cmath>
#include <algorithm>

// BrakePressed (1 bit at 24)
// TransmissionState (3 bits at 0)


uint8_t encode_u8(double physical, double factor, double offset) {
    double scaled;
    long min_val, max_val;
    min_val = 0;
    max_val = 255;

    // Encodes the value into the raw representation
    scaled = (physical - offset) / factor;

    // Rounds the value instead of truncating to minimize error
    long raw = std::lround(scaled);

    // Clamps the value in between 0 and 255, 
    raw = std::clamp(raw, min_val,max_val); 
    // Casts to uint8_t
    return static_cast<uint8_t>(raw);
}


void encode_u16_le(uint8_t* data, int start_byte, double physical, double factor, double offset){
    double scaled = (physical - offset) / factor;

    long raw = std::lround(scaled);
    raw = std::clamp(raw, 0L, 65535L);

    // Little endian
    data[start_byte] = static_cast<uint8_t>(raw & 0xFF);
   
    data[start_byte + 1] = static_cast<uint8_t>((raw >> 8) & 0xFF);
}

void set_bit(uint8_t* data, int bit_pos, bool value){

    // Gets the byte
    int byte_index = bit_pos / 8;

    //Gets the specific bit in the byte
    int bit_in_byte = bit_pos % 8;

    if (value){
        data[byte_index] |= (1 << bit_in_byte);
    } else {
        data[byte_index] &= ~(1 << bit_in_byte);
    }
    
}
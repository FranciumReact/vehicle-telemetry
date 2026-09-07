#include <cstdint>
#include <cmath>
#include <algorithm>

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
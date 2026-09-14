#include "can_decode.hpp"
#include <cstdint>



uint32_t extract_bits(const uint8_t* data, int start_bit, int length){
    uint32_t result = 0;
    for (int i = 0; i < length; i++){
        uint32_t byte = (start_bit + i) / 8;
        uint32_t bit = (start_bit + i) % 8;

        uint32_t bit_value = (data[byte] >> bit) & 1;
        if (bit_value) result |= (1u << i);
    }
    return result;
}

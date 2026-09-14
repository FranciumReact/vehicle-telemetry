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

double decode_signal(const SignalSpec& spec, const uint8_t* data) {
    uint32_t raw = extract_bits(data, spec.start_bit, spec.length);
    return raw * spec.factor + spec.offset;
}

std::optional<DecodedFrame> decode_frame(const CanFrame& frame) {
    for (const MessageSpec& msg : get_message_table()) {
        if (msg.id == frame.id) {
            if (msg.dlc != frame.dlc){
                return std::nullopt;
            } else {
                // Builds name to physical value map for the frame
                DecodedFrame result;
                for (const SignalSpec& sig : msg.signals) {
                    // decode_signal applies factor/offset to raw bits
                    result[sig.name] = decode_signal(sig, frame.data);
                }
                // Converts to std::optional
                return result;
            }
        }
    }
    return std::nullopt;   // no matching ID
}
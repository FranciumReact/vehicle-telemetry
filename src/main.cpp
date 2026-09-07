#include <cstdint>
#include <cstdio>

uint8_t encode_u8(double physical, double factor, double offset);

int main() {
    printf("throttle 50.0%% -> 0x%02X\n", encode_u8(50.0, 0.4, 0.0));
    printf("throttle 33.0%% -> 0x%02X\n", encode_u8(33.0, 0.4, 0.0));
    printf("coolant  65 C   -> 0x%02X\n", encode_u8(65.0, 1.0, -40.0));
    printf("clamped  300%%   -> 0x%02X\n", encode_u8(300.0, 0.4, 0.0));
    return 0;
}
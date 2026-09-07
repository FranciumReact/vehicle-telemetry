#include <cstdint>
#include <cstdio>

uint8_t encode_u8(double physical, double factor, double offset);
void encode_u16_le(uint8_t* data, int start_byte, double physical, double factor, double offset);
void set_bit(uint8_t* data, int bit_pos, bool value);
void set_bits(uint8_t* data, int start_bit, int length, uint32_t value);

int main() {
    printf("throttle 50.0%% -> 0x%02X\n", encode_u8(50.0, 0.4, 0.0));
    printf("throttle 33.0%% -> 0x%02X\n", encode_u8(33.0, 0.4, 0.0));
    printf("coolant  65 C   -> 0x%02X\n", encode_u8(65.0, 1.0, -40.0));
    printf("clamped  300%%   -> 0x%02X\n", encode_u8(300.0, 0.4, 0.0));
    
    uint8_t buf[8] = {0};
    encode_u16_le(buf, 0, 2000.0, 0.25, 0.0);
    printf("rpm 2000 -> %02X %02X\n", buf[0], buf[1]);

    
    set_bit(buf, 24, true);
    printf("brake set    -> %02X\n", buf[3]);
    set_bit(buf, 24, false);
    printf("brake clear  -> %02X\n", buf[3]);

    uint8_t buf2[8] = {0};
    set_bits(buf2, 0, 3, 3);        // TransmissionState = Drive
    set_bits(buf2, 3, 16, 250);     // FuelRate = 25.0 L/h (raw 250)
    printf("powertrain -> %02X %02X %02X\n", buf2[0], buf2[1], buf2[2]);


    return 0;
}
#include <cassert>
#include <cstdio>
#include <cmath>
#include "can_decode.hpp"
#include "frame_builder.hpp"

// "==" is inaccurate and unreliable
static bool close_enough(double a, double b, double tol) {
    return std::fabs(a - b) < tol;
}

int main() {
    // Round trip: encode known values, decode them back
    CanFrame f = build_engine_data(2000.0, 50.0, 20.0, 65.0);
    auto d = decode_frame(f);
    assert(d.has_value());
    assert(close_enough(d.value()["EngineRPM"], 2000.0, 0.25));
    assert(close_enough(d.value()["CoolantTemp"], 65.0, 1.0));

    // Unknown ID is rejected
    CanFrame bad = f;
    bad.id = 0x999;
    assert(!decode_frame(bad).has_value());

    // Wrong DLC is rejected
    CanFrame short_frame = f;
    short_frame.dlc = 4;
    assert(!decode_frame(short_frame).has_value());

    printf("all tests passed\n");
    return 0;
}
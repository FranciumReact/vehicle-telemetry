#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "can_decode.hpp"
#include "frame_builder.hpp"
#include "dtc_engine.hpp"
#include <random>

using Catch::Matchers::WithinAbs;

TEST_CASE("engine frame round-trips through encode and decode") {
    CanFrame f = build_engine_data(2000.0, 50.0, 20.0, 65.0);
    auto d = decode_frame(f);

    REQUIRE(d.has_value());

    // Tolerances are one quantisation step: factor 0.25 for RPM,
    // 0.4 for the percentages, 1.0 for coolant.
    CHECK_THAT(d->at("EngineRPM"),   WithinAbs(2000.0, 0.25));
    CHECK_THAT(d->at("ThrottlePos"), WithinAbs(50.0,   0.4));
    CHECK_THAT(d->at("EngineLoad"),  WithinAbs(20.0,   0.4));
    CHECK_THAT(d->at("CoolantTemp"), WithinAbs(65.0,   1.0));
}

TEST_CASE("malformed frames are rejected") {
    CanFrame f = build_engine_data(2000.0, 50.0, 20.0, 65.0);

    SECTION("unknown identifier") {
        f.id = 0x999;
        CHECK_FALSE(decode_frame(f).has_value());
    }

    SECTION("DLC shorter than the spec") {
        f.dlc = 4;
        CHECK_FALSE(decode_frame(f).has_value());
    }

    SECTION("DLC longer than the spec") {
        f.dlc = 9;
        CHECK_FALSE(decode_frame(f).has_value());
    }
}

TEST_CASE("DTC requires sustained condition before confirming") {
    DtcEngine dtc;
    DecodedFrame hot{{"CoolantTemp", 120.0}};   // above the 110 threshold

    SECTION("nine cycles is not enough") {
        for (int i = 0; i < 9; i++) {
            CHECK(dtc.update(hot, i * 0.01).empty());
        }
    }

    SECTION("the tenth cycle confirms the fault") {
        // burn through the first nine
        for (int i = 0; i < 9; i++) dtc.update(hot, i * 0.01);

        auto events = dtc.update(hot, 0.09);
        REQUIRE(events.size() == 1);
        CHECK(events[0].code == "P0001");
        CHECK(events[0].active);
    }

    SECTION("a confirmed fault does not re-fire") {
        // trigger it
        for (int i = 0; i < 10; i++) dtc.update(hot, i * 0.01);

        // still faulting, but the state has not changed
        CHECK(dtc.update(hot, 0.10).empty());
    }
}

TEST_CASE("decoder survives arbitrary payloads") {
    std::mt19937 rng(12345);                    // fixed seed: reproducible
    std::uniform_int_distribution<int> byte(0, 255);
    std::uniform_int_distribution<int> id(0, 0x7FF);
    std::uniform_int_distribution<int> dlc(0, 8);

    for (int n = 0; n < 10000; n++) {
        CanFrame f{};
        f.id  = static_cast<uint32_t>(id(rng));
        f.dlc = static_cast<uint8_t>(dlc(rng));
        for (auto& b : f.data) b = static_cast<uint8_t>(byte(rng));

        // No assertion on the result: garbage in, anything out is fine.
        // What must hold is that this returns at all, without crashing
        // or reading past the end of the payload.
        auto result = decode_frame(f);
        (void)result;
    }

    SUCCEED("10,000 random frames decoded without crashing");
}
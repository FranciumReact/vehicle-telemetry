#include "frame_builder.hpp"
#include "vehicle_model.hpp"
#include "can_decode.hpp"
#include "frame_queue.hpp"
#include "validation.hpp"
#include "rate_validator.hpp"
#include "dtc_engine.hpp"
#include <cstdio>
#include <thread>
#include <chrono>

int main() {
    // Shared between both threads. Handles its own locking internally,
    // so neither thread has to think about mutexes.
    FrameQueue q;

    // PRODUCER THREAD
    // Stands in for the vehicle's ECUs broadcasting onto the bus.
    // [&] captures q by reference so the thread can push into it.
    std::thread producer([&]{
        VehicleState s;

        // 500 ticks at 10 ms each = 5 seconds of simulated driving
        for (int i = 0; i < 500; i++) {
            update_vehicle(s, 0.01);

            // Fault injection: a sustained overheat between ticks 200 and 215.
            // This REPLACES the real reading rather than adding a second frame.
            // Interleaving good and bad frames would reset the DTC engine's
            // fail counter every other cycle, so it would never confirm.
            double coolant = (i >= 200 && i < 215) ? 200.0 : s.coolant_c;

            q.push(build_engine_data(s.rpm, s.throttle_pct,
                                     s.load_pct, coolant));

            // Emit at the 10 ms cycle time the protocol spec defines.
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // No more frames coming. Without this the consumer blocks forever.
        q.shutdown();
    });

    // CONSUMER THREAD
    // Stands in for the telemetry gateway reading the bus.
    std::thread consumer([&]{
        int decoded = 0, out_of_range = 0, bad_rate = 0;
        RateValidator rv;
        DtcEngine dtc;
        double sim_time = 0.0;

        while (auto frame = q.pop()) {
            auto values = decode_frame(frame.value());
            if (!values) continue;   // unknown ID or DLC mismatch

            decoded++;
            sim_time += 0.01;        // each frame is one 10 ms tick

            // Validation needs the spec, not just the value, so look up
            // which message this frame belongs to.
            for (const MessageSpec& msg : get_message_table()) {
                if (msg.id != frame->id) continue;

                for (const SignalSpec& sig : msg.signals) {
                    double v = values->at(sig.name);

                    // Is this reading physically possible?
                    if (validate_signal(sig, v) == ValidationResult::OutOfRange)
                        out_of_range++;

                    // Did it change faster than the real world allows?
                    if (rv.check(sig, v, sim_time) == ValidationResult::ImplausibleRate)
                        bad_rate++;
                }
            }

            // Diagnostics run on the values themselves. Only state changes
            // come back as events, so a fault that persists prints once.
            for (const DtcEvent& e : dtc.update(*values, sim_time)) {
                printf("  DTC %s %s at t=%.2f\n",
                       e.code.c_str(), e.active ? "SET" : "CLEARED", e.time_s);
            }
        }

        printf("decoded %d, out of range %d, implausible rate %d, dropped %llu\n",
               decoded, out_of_range, bad_rate, (unsigned long long)q.dropped());
    });

    // Wait for both threads to finish. Skipping this would let main
    // return while they're still running, which terminates the program.
    producer.join();
    consumer.join();
    return 0;
}
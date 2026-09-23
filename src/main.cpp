#include "frame_builder.hpp"
#include "vehicle_model.hpp"
#include "can_decode.hpp"
#include "frame_queue.hpp"
#include "validation.hpp"
#include "rate_validator.hpp"
#include "dtc_engine.hpp"
#include "telemetry_writer.hpp"
#include <cstdio>
#include <thread>
#include <chrono>
#include <cstdlib>

int main() {
    // Line-buffer stdout so logs appear immediately when captured by Docker,
    // rather than all at once on exit.
    setvbuf(stdout, nullptr, _IOLBF, 0);

    // Shared between both threads. Handles its own locking internally,
    // so neither thread has to think about mutexes.
    FrameQueue q;

    // PRODUCER THREAD
    // Stands in for the vehicle's ECUs broadcasting onto the bus.
    // [&] captures q by reference so the thread can push into it.
    std::thread producer([&]{
        VehicleState s;

        // Absolute deadlines, not fixed sleeps. sleep_for waits at least
        // 10 ms and then the loop does work on top, so each iteration costs
        // slightly more than 10 ms and the error accumulates — about 0.9 s
        // of drift over a 30-second run. sleep_until absorbs the work time
        // instead, which is how real-time loops are written.
        auto next = std::chrono::steady_clock::now();

        // 3000 ticks at 10 ms each = 30 seconds of simulated driving.
        for (int i = 0; i < 3000; i++) {
            update_vehicle(s, 0.01);

            // Fault injection: a sustained overheat between ticks 1500 and
            // 1700. This REPLACES the real reading rather than adding a
            // second frame — interleaving good and bad frames would reset
            // the DTC engine's fail counter every other cycle, so it would
            // never confirm.
            double coolant = (i >= 1500 && i < 1700) ? 200.0 : s.coolant_c;

            q.push(build_engine_data(s.rpm, s.throttle_pct,
                                     s.load_pct, coolant));

            next += std::chrono::milliseconds(10);
            std::this_thread::sleep_until(next);
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

        // Connection string comes from the environment so a deployment can
        // override it without a code change; falls back to the local default.
        const char* conn = std::getenv("TELEMETRY_DB");
        TelemetryWriter writer(conn ? conn : "dbname=vehicle_telemetry");
        writer.start_session("SIM-001");

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
                writer.log_dtc(e.code, e.active, e.time_s);
            }

            writer.add(*values, sim_time);
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
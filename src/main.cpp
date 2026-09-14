#include "frame_builder.hpp"
#include "vehicle_model.hpp"
#include "can_decode.hpp"
#include "frame_queue.hpp"
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

            q.push(build_engine_data(s.rpm, s.throttle_pct,
                                     s.load_pct, s.coolant_c));

            // Emit at the 10 ms cycle time the protocol spec defines,
            // instead of running flat out. Without this the producer
            // outruns the consumer and the queue drops most frames.
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // No more frames coming. Without this the consumer would
        // block in pop() forever waiting for data that never arrives.
        q.shutdown();
    });

    // CONSUMER THREAD
    // Stands in for the telemetry gateway reading the bus.
    std::thread consumer([&]{
        int decoded = 0;

        // steady_clock never jumps backwards, unlike system_clock,
        // so it's the right choice for measuring elapsed time.
        auto start = std::chrono::steady_clock::now();

        // pop() blocks while the queue is empty. It returns nullopt
        // only once shutdown has been called AND the queue is drained.
        while (auto frame = q.pop()) {
            if (decode_frame(frame.value())) decoded++;
        }

        auto end = std::chrono::steady_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                      end - start).count();

        // NOTE: with the producer throttled to real time, this figure
        // reflects the producer's rate, not the decoder's capacity.
        // The capacity number comes from an unthrottled run.
        printf("decoded %d frames in %lld us (%.0f frames/sec), dropped %llu\n",
               decoded, (long long)us, decoded * 1000000.0 / us,
               (unsigned long long)q.dropped());
    });

    // Wait for both threads to finish. Skipping this would let main
    // return while they're still running, which terminates the program.
    producer.join();
    consumer.join();
    return 0;
}
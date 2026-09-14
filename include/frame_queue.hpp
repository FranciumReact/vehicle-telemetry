#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>
#include "can_frame.hpp"

class FrameQueue {
public:
    void push(const CanFrame& f);
    std::optional<CanFrame> pop();
    void shutdown();
    uint64_t dropped() const;

private:
    std::queue<CanFrame>    queue_;
    mutable std::mutex      mutex_;
    std::condition_variable cv_;
    std::atomic<bool>       done_{false};
    std::atomic<uint64_t>   dropped_{0};
    static constexpr size_t kMaxSize = 1000;
};
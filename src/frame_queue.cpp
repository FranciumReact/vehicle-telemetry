#include "frame_queue.hpp"

void FrameQueue::push(const CanFrame& f) {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (queue_.size() >= kMaxSize) {
            queue_.pop();        // drop oldest
            dropped_++;
        }
        queue_.push(f);
    }
    cv_.notify_one();
}
std::optional<CanFrame> FrameQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    // wait until there's something, or we're shutting down
    cv_.wait(lock, [this]{ return !queue_.empty() || done_; });

    // if nothing left and we're done, signal end of stream
    if (queue_.empty()) return std::nullopt;

    CanFrame f = queue_.front();
    queue_.pop();
    return f;
}

void FrameQueue::shutdown() {
    done_ = true;
    cv_.notify_all();
}

uint64_t FrameQueue::dropped() const {
    return dropped_;
}
#pragma once

#include "types.hpp"
#include <boost/asio.hpp>
#include <condition_variable>
#include <queue>

namespace shortener {
template <typename T> class ThreadSafeQueue {
private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;

public:
  void push(T value) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(std::move(value));
    }

    cv_.notify_one();
  }

  T pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    cv_.wait(lock, [&queue = queue_]() { return !queue.empty(); });

    T value = std::move(queue_.front());
    queue_.pop();

    return value;
  }
};
} // namespace shortener

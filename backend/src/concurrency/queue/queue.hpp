#pragma once

#include "types.hpp"
#include <boost/asio.hpp>
#include <condition_variable>
#include <queue>
#include <optional>

namespace shortener {
template <typename T> class ThreadSafeQueue final {
private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool closed_{false};

public:
  ThreadSafeQueue() = default;

  ThreadSafeQueue(const ThreadSafeQueue &) = delete;
  ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;

  void push(T value) {
    {
      std::lock_guard<std::mutex> lock(mutex_);

      if (closed_) {
        throw std::runtime_error("push() on closed queue");
      }

      queue_.push(std::move(value));
    }

    cv_.notify_one();
  }

  std::optional<T> pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    cv_.wait(lock, [this]() { return closed_ || !queue_.empty(); });

    if (queue_.empty()) {
      return std::nullopt;
    }

    T value = std::move(queue_.front());
    queue_.pop();

    return value;
  }

  void close() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      closed_ = true;
    }

    cv_.notify_all();
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);

    return queue_.empty();
  }

  bool closed() const {
    std::lock_guard<std::mutex> lock(mutex_);

    return closed_;
  }

  std::size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);

    return queue_.size();
  }
};
} // namespace shortener

#pragma once

#include <boost/asio.hpp>
#include <condition_variable>
#include <queue>

using tcp = boost::asio::ip::tcp;

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

    cv_.wait(lock, [this]() { return !queue_.empty(); });

    T value = std::move(queue_.front());
    queue_.pop();

    return value;
  }
};

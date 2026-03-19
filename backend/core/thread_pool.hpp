#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  ThreadPool(size_t threads = 4) {
    for (size_t i = 0; i < threads; ++i) {
      workers_.emplace_back([this]() {
        while (true) {
          std::function<void()> task;

          {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock,
                            [this]() { return stop_ || !tasks_.empty(); });

            if (stop_ && tasks_.empty())
              return;

            task = std::move(tasks_.front());
            tasks_.pop();
          }

          task();
        }
      });
    }
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      stop_ = true;
    }

    condition_.notify_all();

    for (auto &t : workers_)
      t.join();
  }

  template <class F> void enqueue(F &&f) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      tasks_.emplace(std::forward<F>(f));
    }
    condition_.notify_one();
  }

private:
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;

  std::mutex mutex_;
  std::condition_variable condition_;
  bool stop_ = false;
};

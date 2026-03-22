#pragma once

#include "queue.hpp"
#include "shortener.hpp"
#include "types.hpp"
#include <boost/asio.hpp>
#include <thread>
#include <utility>
#include <vector>

namespace shortener {
class ThreadPool final {
private:
  std::vector<std::thread> workers_;
  ThreadSafeQueue<tcp::socket> &socket_queue_;
  UrlShortener &shortener_;
  std::atomic<bool> stopping_{false};

  void worker_loop();

public:
  ThreadPool(ushort num_threads, ThreadSafeQueue<tcp::socket> &queue,
             UrlShortener &shortener);

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

  ~ThreadPool();

  void shutdown();
};
} // namespace shortener
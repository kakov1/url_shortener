#pragma once

#include "concurrency/queue/queue.hpp"
#include "core/types.hpp"
#include "logic/url_service.hpp"
#include <atomic>
#include <boost/asio.hpp>
#include <thread>
#include <utility>
#include <vector>

namespace shortener {
class ThreadPool final {
public:
  using SocketHandler = std::function<void(tcp::socket)>;

private:
  std::vector<std::thread> workers_;
  ThreadSafeQueue<tcp::socket> &socket_queue_;
  SocketHandler handler_;
  std::atomic<bool> stopping_{false};

  void worker_loop();

public:
  ThreadPool(ushort num_threads, ThreadSafeQueue<tcp::socket> &queue,
             SocketHandler handler);

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

  ~ThreadPool();

  void shutdown();
};
} // namespace shortener
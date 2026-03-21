#pragma once

#include "shortener.hpp"
#include "queue.hpp"
#include "types.hpp"
#include <boost/asio.hpp>
#include <thread>
#include <utility>
#include <vector>

namespace shortener {
class ThreadPool {
private:
  std::vector<std::thread> workers_;
  ThreadSafeQueue<tcp::socket> &queue_;
  io_context &io_context_;
  UrlShortener &shortener_;

public:
  ThreadPool(ushort num_threads, ThreadSafeQueue<tcp::socket> &queue,
             io_context &io_context, UrlShortener &shortener);

  ~ThreadPool();
};
} // namespace shortener
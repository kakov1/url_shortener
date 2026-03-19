#pragma once

#include "../shortener/shortener.hpp"
#include "safequeue.hpp"
#include <boost/asio.hpp>
#include <thread>
#include <utility>
#include <vector>

using boost::asio::ip::tcp;

class ThreadPool {
private:
  std::vector<std::thread> workers_;
  ThreadSafeQueue<tcp::socket> &queue_;
  boost::asio::io_context &io_context_;
  UrlShortener &shortener_;

public:
  ThreadPool(size_t num_threads, ThreadSafeQueue<tcp::socket> &queue,
             boost::asio::io_context &io_context, UrlShortener &shortener);

  ~ThreadPool();
};
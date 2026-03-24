#pragma once

#include "logic/url_service.hpp"
#include "concurrency/threadpool/threadpool.hpp"
#include "entities/entities.hpp"
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <memory>
#include <vector>
#include <atomic>

namespace shortener {
class HttpServer final {
private:
  io_context &io_context_;
  tcp::acceptor acceptor_;
  ThreadSafeQueue<tcp::socket> socket_queue_;
  ThreadPool thread_pool_;
  std::atomic<bool> is_running_;

  void accept_loop();

public:
  HttpServer(io_context &io_context, ushort port, ushort num_threads,
             UrlService &url_service);

  void run();
  void stop();

  ~HttpServer();
};
} // namespace shortener
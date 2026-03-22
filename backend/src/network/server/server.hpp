#pragma once

#include "in_memory_url_repository.hpp"
#include "threadpool.hpp"
#include "types.hpp"
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <memory>
#include <vector>

namespace shortener {
class HttpServer {
private:
  io_context &io_context_;
  tcp::acceptor acceptor_;
  ThreadSafeQueue<tcp::socket> socket_queue_;
  ThreadPool thread_pool_;
  std::atomic<bool> is_running_;

  void accept_loop();

public:
  HttpServer(io_context &io_context, ushort port, ushort num_threads,
             UrlService &url_service_);

  void run();
  void stop();

  ~HttpServer();
};
} // namespace shortener
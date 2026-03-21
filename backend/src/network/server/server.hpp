#pragma once

#include "queue.hpp"
#include "threadpool.hpp"
#include "types.hpp"
#include "shortener.hpp"
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <memory>
#include <vector>

namespace shortener {
class HttpSession {
private:
  tcp::socket socket_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  UrlShortener &shortener_;

public:
  HttpSession(tcp::socket socket, UrlShortener &shortener);

  void handle();
};

class HttpServer {
private:
  io_context &io_context_;
  tcp::acceptor acceptor_;
  ThreadSafeQueue<tcp::socket> socket_queue_;
  ThreadPool thread_pool_;
  UrlShortener shortener_;

  void accept();

public:
  HttpServer(io_context &io_context, ushort port, ushort num_threads);

  void run();
};
} // namespace shortener
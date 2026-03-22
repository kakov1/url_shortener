#include "server.hpp"
#include <iostream>

namespace shortener {

HttpServer::HttpServer(io_context &io_context, ushort port, ushort num_threads)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), shortener_(),
      thread_pool_(num_threads, socket_queue_, shortener_),
      is_running_{false} {}

void HttpServer::run() {
  if (is_running_.exchange(true)) {
    return;
  }

  std::cout << "HTTP server started on port "
            << acceptor_.local_endpoint().port() << std::endl;

  accept_loop();
}

void HttpServer::stop() {
  if (!is_running_.exchange(false)) {
    return;
  }

  boost::system::error_code ec;
  acceptor_.close(ec);

  if (ec) {
    std::cerr << "Failed to close acceptor: " << ec.message() << std::endl;
  }

  socket_queue_.close();
  thread_pool_.shutdown();

  std::cout << "HTTP server stopped" << std::endl;
}

void HttpServer::accept_loop() {
  while (is_running_) {
    boost::system::error_code ec;
    tcp::socket socket(io_context_);

    acceptor_.accept(socket, ec);

    if (ec) {
      if (!is_running_) {
        break;
      }

      std::cerr << "Accept error: " << ec.message() << std::endl;
      continue;
    }

    try {
      socket_queue_.push(std::move(socket));
    } catch (const std::exception &e) {
      std::cerr << "Failed to push socket: " << e.what() << std::endl;
    }
  }
}

HttpServer::~HttpServer() { stop(); }
} // namespace shortener
#include "server.hpp"
#include "network/session/http_session.hpp"
#include <iostream>

namespace shortener {

HttpServer::HttpServer(ushort port, ushort num_threads, UrlService &url_service)
    : io_context_(), acceptor_(io_context_),
      thread_pool_(num_threads, socket_queue_,
                   [&url_service, port](tcp::socket socket) {
                     std::unique_ptr<ISession> session =
                         std::make_unique<HttpSession>(std::move(socket),
                                                       url_service, port);
                     session->handle_session();
                   }),
      is_running_{false} {
  boost::system::error_code ec;

  acceptor_.open(tcp::v4(), ec);

  if (ec)
    throw std::runtime_error("failed to open acceptor: " + ec.message());

  acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);

  if (ec)
    throw std::runtime_error("failed to set reuse_address: " + ec.message());

  acceptor_.bind(tcp::endpoint(tcp::v4(), port), ec);

  if (ec)
    throw std::runtime_error("failed to bind acceptor: " + ec.message());

  acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);

  if (ec)
    throw std::runtime_error("failed to listen: " + ec.message());
}

void HttpServer::run() {
  if (is_running_.exchange(true)) {
    return;
  }

  std::cout << "HTTP server started on port "
            << acceptor_.local_endpoint().port() << std::endl;

  try {
    accept_loop();
  } catch (...) {
    stop();
    throw;
  }
}

void HttpServer::stop() {
  if (!is_running_.exchange(false)) {
    return;
  }

  boost::system::error_code ec;

  acceptor_.cancel(ec);

  if (ec)
    std::cerr << "Failed to cancel acceptor: " << ec.message() << std::endl;

  ec.clear();
  acceptor_.close(ec);

  if (ec)
    std::cerr << "Failed to close acceptor: " << ec.message() << std::endl;

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

      boost::system::error_code shutdown_ec;
      socket.shutdown(tcp::socket::shutdown_both, shutdown_ec);

      boost::system::error_code close_ec;
      socket.close(close_ec);
    }
  }
}

HttpServer::~HttpServer() { stop(); }
} // namespace shortener
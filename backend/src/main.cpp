#include "network/server.hpp"
#include <memory>

int main() {
  boost::asio::io_context io_context;

  const size_t THREADS = 6;
  HttpServer server(io_context, 8080, THREADS);
}
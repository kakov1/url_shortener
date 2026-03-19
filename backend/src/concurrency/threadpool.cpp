#include "threadpool.hpp"
#include "../network/session.hpp"

ThreadPool::ThreadPool(size_t num_threads, ThreadSafeQueue<tcp::socket> &queue,
                       boost::asio::io_context &io_context,
                       UrlShortener &shortener)
    : queue_(queue), io_context_(io_context), shortener_(shortener) {

  for (size_t i = 0; i < num_threads; ++i) {
    workers_.emplace_back([&queue = queue_, &shortener = shortener_]() {
      while (true) {
        tcp::socket socket = queue.pop();
        HttpSession session(std::move(socket), shortener);
        session.handle();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  for (auto &t : workers_) {
    if (t.joinable())
      t.join();
  }
}
#include "threadpool.hpp"
#include "session.hpp"
#include <iostream>

namespace shortener {
ThreadPool::ThreadPool(ushort num_threads, ThreadSafeQueue<tcp::socket> &queue,
                       UrlShortener &shortener)
    : socket_queue_(queue), shortener_(shortener) {
  workers_.reserve(num_threads);

  for (size_t i = 0; i < num_threads; ++i)
    workers_.emplace_back(&ThreadPool::worker_loop, this);
}

ThreadPool::~ThreadPool() { shutdown(); }

void ThreadPool::shutdown() {
  if (stopping_.exchange(true)) {
    return;
  }

  socket_queue_.close();

  for (auto &worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void ThreadPool::worker_loop() {
  while (true) {
    auto socket_opt = socket_queue_.pop();

    if (!socket_opt.has_value()) {
      break;
    }

    try {
      HttpSession session(std::move(*socket_opt), shortener_);
      session.handle();
    } catch (const std::exception &e) {
      std::cerr << "Worker session error: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Worker session error: unknown exception" << std::endl;
    }
  }
}
} // namespace shortener
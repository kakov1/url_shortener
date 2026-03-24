#pragma once

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include <pqxx/pqxx>

namespace shortener {

class PostgresConnectionPool {
private:
  std::vector<std::unique_ptr<pqxx::connection>> connections_;
  std::queue<pqxx::connection *> available_connections_;
  std::mutex mutex_;
  std::condition_variable condition_variable_;

private:
  void release(pqxx::connection *connection);

public:
  class ConnectionGuard {
  public:
    ConnectionGuard(PostgresConnectionPool &pool, pqxx::connection *connection);
    ~ConnectionGuard();

    ConnectionGuard(const ConnectionGuard &) = delete;
    ConnectionGuard &operator=(const ConnectionGuard &) = delete;

    ConnectionGuard(ConnectionGuard &&other) noexcept;
    ConnectionGuard &operator=(ConnectionGuard &&other) noexcept;

    pqxx::connection &get() const;
    pqxx::connection &operator*() const;
    pqxx::connection *operator->() const;

  private:
    PostgresConnectionPool *pool_;
    pqxx::connection *connection_;
  };

public:
  PostgresConnectionPool(const std::string &connection_string,
                         std::size_t pool_size = 8);

  ConnectionGuard acquire();
};

} // namespace shortener
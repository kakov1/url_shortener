#include "repositories/postgres/postgres_connection_pool.hpp"

#include <stdexcept>
#include <utility>

namespace shortener {

PostgresConnectionPool::ConnectionGuard::ConnectionGuard(
    PostgresConnectionPool &pool, pqxx::connection *connection)
    : pool_(&pool), connection_(connection) {}

PostgresConnectionPool::ConnectionGuard::~ConnectionGuard() {
  if (pool_ != nullptr && connection_ != nullptr) {
    pool_->release(connection_);
  }
}

PostgresConnectionPool::ConnectionGuard::ConnectionGuard(
    ConnectionGuard &&other) noexcept
    : pool_(other.pool_), connection_(other.connection_) {
  other.pool_ = nullptr;
  other.connection_ = nullptr;
}

PostgresConnectionPool::ConnectionGuard &
PostgresConnectionPool::ConnectionGuard::operator=(
    ConnectionGuard &&other) noexcept {
  if (this != &other) {
    if (pool_ != nullptr && connection_ != nullptr) {
      pool_->release(connection_);
    }

    pool_ = other.pool_;
    connection_ = other.connection_;

    other.pool_ = nullptr;
    other.connection_ = nullptr;
  }

  return *this;
}

pqxx::connection &PostgresConnectionPool::ConnectionGuard::get() const {
  return *connection_;
}

pqxx::connection &PostgresConnectionPool::ConnectionGuard::operator*() const {
  return *connection_;
}

pqxx::connection *PostgresConnectionPool::ConnectionGuard::operator->() const {
  return connection_;
}

PostgresConnectionPool::PostgresConnectionPool(
    const std::string &connection_string, std::size_t pool_size) {
  if (pool_size == 0) {
    throw std::invalid_argument("pool_size must be greater than zero");
  }

  connections_.reserve(pool_size);

  for (std::size_t i = 0; i < pool_size; ++i) {
    auto connection = std::make_unique<pqxx::connection>(connection_string);

    if (!connection->is_open()) {
      throw std::runtime_error("failed to open PostgreSQL connection");
    }

    available_connections_.push(connection.get());
    connections_.push_back(std::move(connection));
  }
}

PostgresConnectionPool::ConnectionGuard PostgresConnectionPool::acquire() {
  std::unique_lock<std::mutex> lock(mutex_);

  condition_variable_.wait(
      lock, [this]() { return !available_connections_.empty(); });

  pqxx::connection *connection = available_connections_.front();
  available_connections_.pop();

  return ConnectionGuard(*this, connection);
}

void PostgresConnectionPool::release(pqxx::connection *connection) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    available_connections_.push(connection);
  }

  condition_variable_.notify_one();
}

} // namespace shortener

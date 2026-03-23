#include "repositories/postgres/postgres_user_repository.hpp"

#include <stdexcept>

namespace shortener {

PostgresUserRepository::PostgresUserRepository(pqxx::connection &connection)
    : connection_(connection) {}

std::optional<User> PostgresUserRepository::find_by_id(std::int64_t id) const {
  pqxx::read_transaction tx(connection_);

  pqxx::result result =
      tx.exec_params("SELECT id, username FROM users WHERE id = $1", id);

  if (result.empty()) {
    return std::nullopt;
  }

  auto row = result[0];

  return User{.id = row["id"].as<std::int64_t>(),
              .username = row["username"].c_str()};
}

std::optional<User>
PostgresUserRepository::find_by_username(const std::string &username) const {
  pqxx::read_transaction tx(connection_);

  pqxx::result result = tx.exec_params(
      "SELECT id, username FROM users WHERE username = $1", username);

  if (result.empty()) {
    return std::nullopt;
  }

  auto row = result[0];

  return User{.id = row["id"].as<std::int64_t>(),
              .username = row["username"].c_str()};
}

User PostgresUserRepository::create(const std::string &username) {
  pqxx::work tx(connection_);

  pqxx::result result =
      tx.exec_params("INSERT INTO users (username) VALUES ($1) "
                     "RETURNING id, username",
                     username);

  tx.commit();

  if (result.empty()) {
    throw std::runtime_error("failed to create user");
  }

  auto row = result[0];

  return User{.id = row["id"].as<std::int64_t>(),
              .username = row["username"].c_str()};
}

} // namespace shortener

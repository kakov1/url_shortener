#include "repositories/postgres/users/postgres_user_repository.hpp"

#include <stdexcept>

#include <pqxx/pqxx>

namespace shortener {

PostgresUserRepository::PostgresUserRepository(
    PostgresConnectionPool &connection_pool)
    : connection_pool_(connection_pool) {}

std::optional<User> PostgresUserRepository::find_by_id(std::int64_t id) const {
  auto connection = connection_pool_.acquire();
  pqxx::read_transaction tx(connection.get());

  const pqxx::result result =
      tx.exec_params("SELECT id, username FROM users WHERE id = $1", id);

  if (result.empty()) {
    return std::nullopt;
  }

  const auto row = result[0];
  return User{.id = row["id"].as<std::int64_t>(),
              .username = row["username"].c_str()};
}

std::optional<User>
PostgresUserRepository::find_by_username(const std::string &username) const {
  auto conn = connection_pool_.acquire();
  pqxx::work tx(*conn);

  pqxx::result r = tx.exec_params(
      "SELECT id, username FROM users WHERE username = $1",
      username);

  if (r.empty()) {
    return std::nullopt;
  }

  return User{
      r[0]["id"].as<std::int64_t>(),
      r[0]["username"].as<std::string>()
  };
}

User PostgresUserRepository::create(const std::string &username) {
  auto connection = connection_pool_.acquire();
  pqxx::work tx(connection.get());

  const pqxx::result result =
      tx.exec_params("INSERT INTO users (username) VALUES ($1) "
                     "RETURNING id, username",
                     username);

  tx.commit();

  if (result.empty()) {
    throw std::runtime_error("failed to create user");
  }

  const auto row = result[0];
  return User{.id = row["id"].as<std::int64_t>(),
              .username = row["username"].c_str()};
}

} // namespace shortener
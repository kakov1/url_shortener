#include "repositories/postgres/urls/postgres_url_repository.hpp"

#include <stdexcept>

#include <pqxx/pqxx>

namespace shortener {

PostgresUrlRepository::PostgresUrlRepository(
    PostgresConnectionPool &connection_pool)
    : connection_pool_(connection_pool) {}

std::optional<Url> PostgresUrlRepository::find_by_id(std::int64_t id) const {
  auto connection = connection_pool_.acquire();
  pqxx::read_transaction tx(connection.get());

  const pqxx::result result =
      tx.exec_params("SELECT id, original_url, short_key, user_id, created_at "
                     "FROM urls WHERE id = $1",
                     id);

  if (result.empty()) {
    return std::nullopt;
  }

  return map_row_to_url(result[0]);
}

std::optional<Url>
PostgresUrlRepository::find_by_short_key(const std::string &short_key) const {
  auto connection = connection_pool_.acquire();
  pqxx::read_transaction tx(connection.get());

  const pqxx::result result =
      tx.exec_params("SELECT id, original_url, short_key, user_id, created_at "
                     "FROM urls WHERE short_key = $1",
                     short_key);

  if (result.empty()) {
    return std::nullopt;
  }

  return map_row_to_url(result[0]);
}

std::optional<Url> PostgresUrlRepository::find_by_original_url(
    const std::string &original_url) const {
  auto connection = connection_pool_.acquire();
  pqxx::read_transaction tx(connection.get());

  const pqxx::result result =
      tx.exec_params("SELECT id, original_url, short_key, user_id, created_at "
                     "FROM urls WHERE original_url = $1 "
                     "ORDER BY id ASC LIMIT 1",
                     original_url);

  if (result.empty()) {
    return std::nullopt;
  }

  return map_row_to_url(result[0]);
}

bool PostgresUrlRepository::exists_by_short_key(
    const std::string &short_key) const {
  auto connection = connection_pool_.acquire();
  pqxx::read_transaction tx(connection.get());

  const pqxx::result result = tx.exec_params(
      "SELECT 1 FROM urls WHERE short_key = $1 LIMIT 1", short_key);

  return !result.empty();
}

Url PostgresUrlRepository::create(const std::string &original_url,
                                  const std::string &short_key,
                                  const std::optional<std::int64_t> &user_id) {
  auto connection = connection_pool_.acquire();
  pqxx::work tx(connection.get());

  pqxx::result result;

  if (user_id.has_value()) {
    result = tx.exec_params(
        "INSERT INTO urls (original_url, short_key, user_id) "
        "VALUES ($1, $2, $3) "
        "RETURNING id, original_url, short_key, user_id, created_at",
        original_url, short_key, *user_id);
  } else {
    result = tx.exec_params(
        "INSERT INTO urls (original_url, short_key, user_id) "
        "VALUES ($1, $2, NULL) "
        "RETURNING id, original_url, short_key, user_id, created_at",
        original_url, short_key);
  }

  tx.commit();

  if (result.empty()) {
    throw std::runtime_error("failed to create url");
  }

  return map_row_to_url(result[0]);
}

std::vector<Url>
PostgresUrlRepository::find_all_by_user_id(std::int64_t user_id) const {
  auto connection = connection_pool_.acquire();
  pqxx::read_transaction tx(connection.get());

  const pqxx::result result =
      tx.exec_params("SELECT id, original_url, short_key, user_id, created_at "
                     "FROM urls WHERE user_id = $1 "
                     "ORDER BY id ASC",
                     user_id);

  std::vector<Url> urls;
  urls.reserve(result.size());

  for (const auto &row : result) {
    urls.push_back(map_row_to_url(row));
  }

  return urls;
}

Url PostgresUrlRepository::map_row_to_url(const pqxx::row &row) {
  std::optional<std::int64_t> user_id = std::nullopt;
  if (!row["user_id"].is_null()) {
    user_id = row["user_id"].as<std::int64_t>();
  }

  return Url{.id = row["id"].as<std::int64_t>(),
             .original_url = row["original_url"].c_str(),
             .short_key = row["short_key"].c_str(),
             .user_id = user_id,
             .created_at = row["created_at"].c_str()};
}

} // namespace shortener

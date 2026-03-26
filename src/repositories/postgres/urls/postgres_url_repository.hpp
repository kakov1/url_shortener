#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <pqxx/pqxx>

#include "entities/entities.hpp"
#include "repositories/postgres/postgres_connection_pool.hpp"
#include "repositories/url_repository.hpp"

namespace shortener {

class PostgresUrlRepository final : public IUrlRepository {
private:
  PostgresConnectionPool &connection_pool_;

private:
  static Url map_row_to_url(const pqxx::row &row);

public:
  explicit PostgresUrlRepository(PostgresConnectionPool &connection_pool);
  ~PostgresUrlRepository() override = default;

  std::optional<Url> find_by_id(std::int64_t id) const override;
  std::optional<Url>
  find_by_short_key(const std::string &short_key) const override;

  std::optional<Url>
  find_public_by_original_url(const std::string &original_url) const override;

  std::optional<Url>
  find_by_original_url_and_user_id(const std::string &original_url,
                                   std::int64_t user_id) const override;

  bool exists_by_short_key(const std::string &short_key) const override;

  Url create(const std::string &original_url, const std::string &short_key,
             const std::optional<std::int64_t> &user_id) override;

  std::vector<Url> find_all_by_user_id(std::int64_t user_id) const override;
};

} // namespace shortener
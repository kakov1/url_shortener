#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <pqxx/pqxx>

#include "entities/entities.hpp"
#include "repositories/user_repository.hpp"

namespace shortener {

class PostgresUserRepository final : public IUserRepository {
private:
  pqxx::connection &connection_;

public:
  explicit PostgresUserRepository(pqxx::connection &connection);
  ~PostgresUserRepository() override = default;

  std::optional<User> find_by_id(std::int64_t id) const override;
  std::optional<User>
  find_by_username(const std::string &username) const override;
  User create(const std::string &username) override;
};

} // namespace shortener
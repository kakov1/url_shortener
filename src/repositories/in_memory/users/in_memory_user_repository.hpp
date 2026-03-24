#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "entities/entities.hpp"
#include "repositories/user_repository.hpp"

namespace shortener {

class InMemoryUserRepository final : public IUserRepository {
private:
  mutable std::mutex mutex_;
  std::vector<User> users_;
  std::int64_t next_id_{1};

public:
  InMemoryUserRepository() = default;
  ~InMemoryUserRepository() override = default;

  std::optional<User> find_by_id(std::int64_t id) const override;
  std::optional<User>
  find_by_username(const std::string &username) const override;
  User create(const std::string &username) override;
};

} // namespace shortener
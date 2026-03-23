#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "entities/entities.hpp"

namespace shortener {

class IUserRepository {
public:
  virtual ~IUserRepository() = default;

  virtual std::optional<User> find_by_id(std::int64_t id) const = 0;
  virtual std::optional<User>
  find_by_username(const std::string &username) const = 0;

  virtual User create(const std::string &username) = 0;
};

} // namespace shortener
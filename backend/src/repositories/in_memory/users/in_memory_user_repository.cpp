#include "in_memory_user_repository.hpp"

namespace shortener {

std::optional<User> InMemoryUserRepository::find_by_id(std::int64_t id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  for (auto &&user : users_) {
    if (user.id == id) {
      return user;
    }
  }

  return std::nullopt;
}

std::optional<User>
InMemoryUserRepository::find_by_username(const std::string &username) const {
  std::lock_guard<std::mutex> lock(mutex_);

  for (auto &&user : users_) {
    if (user.username == username) {
      return user;
    }
  }

  return std::nullopt;
}

User InMemoryUserRepository::create(const std::string &username) {
  std::lock_guard<std::mutex> lock(mutex_);

  for (auto &&user : users_) {
    if (user.username == username) {
      return user;
    }
  }

  User user{.id = next_id_++, .username = username};

  users_.push_back(std::move(user));

  return users_.back();
}

} // namespace shortener
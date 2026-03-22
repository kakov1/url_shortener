#include "in_memory_url_repository.hpp"

namespace shortener {

std::optional<std::string> InMemoryUrlRepository::find_original_by_short_key(
    const std::string &short_key) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = short_to_original_.find(short_key);

  if (it == short_to_original_.end()) {
    return std::nullopt;
  }

  return it->second;
}

std::optional<std::string> InMemoryUrlRepository::find_short_key_by_original(
    const std::string &original_url) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = original_to_short_.find(original_url);

  if (it == original_to_short_.end()) {
    return std::nullopt;
  }

  return it->second;
}

void InMemoryUrlRepository::save(const std::string &short_key,
                                 const std::string &original_url) {
  std::lock_guard<std::mutex> lock(mutex_);

  short_to_original_[short_key] = original_url;
  original_to_short_[original_url] = short_key;
}

} // namespace shortener

#pragma once

#include "url_repository.hpp"

#include <mutex>
#include <string>
#include <unordered_map>

namespace shortener {

class InMemoryUrlRepository final : public IUrlRepository {
private:
  mutable std::mutex mutex_;
  std::unordered_map<std::string, std::string> short_to_original_;
  std::unordered_map<std::string, std::string> original_to_short_;

public:
  InMemoryUrlRepository() = default;

  InMemoryUrlRepository(const InMemoryUrlRepository &) = delete;
  InMemoryUrlRepository &operator=(const InMemoryUrlRepository &) = delete;

  std::optional<std::string>
  find_original_by_short_key(const std::string &short_key) const override;

  std::optional<std::string>
  find_short_key_by_original(const std::string &original_url) const override;

  void save(const std::string &short_key,
            const std::string &original_url) override;
};

} // namespace shortener
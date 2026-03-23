#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "entities/entities.hpp"
#include "repositories/url_repository.hpp"

namespace shortener {

class InMemoryUrlRepository final : public IUrlRepository {
private:
  mutable std::mutex mutex_;
  std::vector<Url> urls_;
  std::int64_t next_id_{1};

private:
  std::string make_created_at() const;

public:
  InMemoryUrlRepository() = default;
  ~InMemoryUrlRepository() override = default;

  std::optional<Url> find_by_id(std::int64_t id) const override;
  std::optional<Url>
  find_by_short_key(const std::string &short_key) const override;
  std::optional<Url>
  find_by_original_url(const std::string &original_url) const override;

  bool exists_by_short_key(const std::string &short_key) const override;

  Url create(const std::string &original_url, const std::string &short_key,
             const std::optional<std::int64_t> &user_id) override;

  std::vector<Url> find_all_by_user_id(std::int64_t user_id) const override;
};

} // namespace shortener
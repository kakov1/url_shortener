#pragma once

#include "entities/entities.hpp"
#include <optional>
#include <string>
#include <vector>

namespace shortener {

class IUrlRepository {
public:
  virtual ~IUrlRepository() = default;

  virtual std::optional<Url> find_by_id(std::int64_t id) const = 0;
  virtual std::optional<Url>
  find_by_short_key(const std::string &short_key) const = 0;

  virtual std::optional<Url>
  find_public_by_original_url(const std::string &original_url) const = 0;

  virtual std::optional<Url>
  find_by_original_url_and_user_id(const std::string &original_url,
                                   std::int64_t user_id) const = 0;

  virtual bool exists_by_short_key(const std::string &short_key) const = 0;

  virtual Url create(const std::string &original_url,
                     const std::string &short_key,
                     const std::optional<std::int64_t> &user_id) = 0;

  virtual std::vector<Url> find_all_by_user_id(std::int64_t user_id) const = 0;
};

} // namespace shortener
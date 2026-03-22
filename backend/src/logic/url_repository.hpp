#pragma once

#include <optional>
#include <string>

namespace shortener {

class IUrlRepository {
public:
  virtual ~IUrlRepository() = default;

  virtual std::optional<std::string>
  find_original_by_short_key(const std::string &short_key) const = 0;

  virtual std::optional<std::string>
  find_short_key_by_original(const std::string &original_url) const = 0;

  virtual void save(const std::string &short_key,
                    const std::string &original_url) = 0;
};

} // namespace shortener
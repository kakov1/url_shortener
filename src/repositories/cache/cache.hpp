#pragma once

#include <cstddef>
#include <optional>
#include <string>

namespace shortener {

class IUrlCache {
public:
  virtual ~IUrlCache() = default;

  virtual std::optional<std::string>
  get_original_url(const std::string &short_key) = 0;

  virtual void put_original_url(const std::string &short_key,
                                const std::string &original_url,
                                std::size_t ttl_seconds) = 0;

  virtual void remove_original_url(const std::string &short_key) = 0;
};

} // namespace shortener
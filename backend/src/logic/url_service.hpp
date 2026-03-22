#pragma once

#include "url_repository.hpp"
#include <string>

namespace shortener {

class UrlService final {
private:
  IUrlRepository &repository_;

  std::string generate_short_key(const std::string &original_url,
                                 std::size_t attempt = 0) const;
  std::string encode_base62(std::size_t value) const;

public:
  explicit UrlService(IUrlRepository &repository);

  std::string shorten(const std::string &original_url);
  std::string restore(const std::string &short_key) const;
};

} // namespace shortener
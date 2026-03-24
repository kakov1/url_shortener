#pragma once

#include "repositories/url_repository.hpp"
#include "repositories/user_repository.hpp"
#include <string>

namespace shortener {

class UrlService {
private:
  IUrlRepository &url_repository_;
  IUserRepository &user_repository_;

private:
  std::string generate_short_key(const std::string &original_url) const;
  std::string encode_base62(std::uint64_t value) const;
  std::string build_candidate_key(const std::string &original_url,
                                  std::uint64_t attempt) const;

public:
  UrlService(IUrlRepository &url_repository, IUserRepository &user_repository);

  User create_user(const std::string &username);

  Url shorten_url(const std::string &original_url,
                  const std::optional<std::int64_t> &user_id = std::nullopt);

  std::optional<std::string> resolve_url(const std::string &short_key) const;

  std::vector<Url> get_user_urls(std::int64_t user_id) const;
};

} // namespace shortener
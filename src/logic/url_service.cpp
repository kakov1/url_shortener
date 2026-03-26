#include "url_service.hpp"

#include <functional>
#include <stdexcept>
#include <utility>

#include <xxhash.h>

namespace shortener {

UrlService::UrlService(IUrlRepository &url_repository,
                       IUserRepository &user_repository)
    : url_repository_(url_repository), user_repository_(user_repository) {}

User UrlService::create_user(const std::string &username) {
  if (username.empty()) {
    throw std::invalid_argument("username cannot be empty");
  }

  auto existing_user = user_repository_.find_by_username(username);

  if (existing_user.has_value()) {
    return existing_user.value();
  }

  return user_repository_.create(username);
}

Url UrlService::shorten_url(const std::string &original_url,
                            const std::optional<std::int64_t> &user_id) {
  if (original_url.empty()) {
    throw std::invalid_argument("original_url cannot be empty");
  }

  if (!is_valid_url(original_url)) {
    throw std::invalid_argument(
        "original_url must start with http:// or https://");
  }

  if (user_id.has_value()) {
    auto user = user_repository_.find_by_id(*user_id);

    if (!user.has_value()) {
      throw std::runtime_error("user not found");
    }

    auto existing_url = url_repository_.find_by_original_url_and_user_id(
        original_url, *user_id);

    if (existing_url.has_value()) {
      return *existing_url;
    }
  } else {
    auto existing_public_url =
        url_repository_.find_public_by_original_url(original_url);

    if (existing_public_url.has_value()) {
      return *existing_public_url;
    }
  }

  constexpr std::uint64_t max_attempts = 100;

  for (std::uint64_t attempt = 0; attempt < max_attempts; ++attempt) {
    const std::string short_key = build_candidate_key(original_url, attempt);

    if (!url_repository_.exists_by_short_key(short_key)) {
      return url_repository_.create(original_url, short_key, user_id);
    }
  }

  throw std::runtime_error("failed to generate unique short key");
}

std::optional<std::string>
UrlService::resolve_url(const std::string &short_key) const {
  if (short_key.empty()) {
    return std::nullopt;
  }

  auto url = url_repository_.find_by_short_key(short_key);

  if (!url.has_value()) {
    return std::nullopt;
  }

  return url->original_url;
}

std::vector<Url> UrlService::get_user_urls(std::int64_t user_id) const {
  auto user = user_repository_.find_by_id(user_id);

  if (!user.has_value()) {
    throw std::runtime_error("user not found");
  }

  return url_repository_.find_all_by_user_id(user_id);
}

std::string
UrlService::generate_short_key(const std::string &original_url) const {
  XXH64_hash_t hash_value =
      XXH64(original_url.data(), original_url.size(), 0);

  std::string encoded = encode_base62(static_cast<std::uint64_t>(hash_value));

  constexpr std::size_t desired_length = 8;

  if (encoded.length() > desired_length) {
    encoded = encoded.substr(0, desired_length);
  }

  return encoded;
}

std::string UrlService::encode_base62(std::uint64_t value) const {
  static constexpr char alphabet[] =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

  if (value == 0) {
    return "0";
  }

  std::string result;

  while (value > 0) {
    result.insert(result.begin(), alphabet[value % 62]);
    value /= 62;
  }

  return result;
}

std::string UrlService::build_candidate_key(const std::string &original_url,
                                            std::uint64_t attempt) const {
  if (attempt == 0) {
    return generate_short_key(original_url);
  }

  return generate_short_key(original_url + "#" + std::to_string(attempt));
}

bool UrlService::is_valid_url(const std::string &url) const {
  return url.rfind("http://", 0) == 0 || url.rfind("https://", 0) == 0;
}

} // namespace shortener
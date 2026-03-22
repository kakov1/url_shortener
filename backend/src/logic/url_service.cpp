#include "url_service.hpp"

#include <functional>
#include <stdexcept>

namespace shortener {

UrlService::UrlService(IUrlRepository &repository) : repository_(repository) {}

std::string UrlService::shorten(const std::string &original_url) {
  if (original_url.empty()) {
    throw std::invalid_argument("original_url must not be empty");
  }

  auto existing = repository_.find_short_key_by_original(original_url);
  if (existing.has_value()) {
    return *existing;
  }

  for (std::size_t attempt = 0; attempt < 32; ++attempt) {
    std::string short_key = generate_short_key(original_url, attempt);

    auto collision = repository_.find_original_by_short_key(short_key);

    if (!collision.has_value()) {
      repository_.save(short_key, original_url);
      return short_key;
    }

    if (*collision == original_url) {
      return short_key;
    }
  }

  throw std::runtime_error("failed to generate unique short key");
}

std::string UrlService::restore(const std::string &short_key) const {
  if (short_key.empty()) {
    return "";
  }

  auto original = repository_.find_original_by_short_key(short_key);

  if (!original.has_value()) {
    return "";
  }

  return *original;
}

std::string UrlService::generate_short_key(const std::string &original_url,
                                           std::size_t attempt) const {
  std::string seed = original_url;
  if (attempt > 0) {
    seed += "#" + std::to_string(attempt);
  }

  std::size_t hash_value = std::hash<std::string>{}(seed);
  std::string encoded = encode_base62(hash_value);

  constexpr std::size_t kKeyLength = 6;

  if (encoded.size() < kKeyLength) {
    encoded.insert(encoded.begin(), kKeyLength - encoded.size(), '0');
  }

  if (encoded.size() > kKeyLength) {
    encoded = encoded.substr(0, kKeyLength);
  }

  return encoded;
}

std::string UrlService::encode_base62(std::size_t value) const {
  static constexpr char alphabet[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  if (value == 0) {
    return "0";
  }

  std::string result;
  while (value > 0) {
    result.push_back(alphabet[value % 62]);
    value /= 62;
  }

  return std::string(result.rbegin(), result.rend());
}

} // namespace shortener

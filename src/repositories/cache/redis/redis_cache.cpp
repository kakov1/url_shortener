#include "redis_cache.hpp"

#include <sw/redis++/redis++.h>
#include <utility>

namespace shortener {

RedisUrlCache::RedisUrlCache(const std::string &host, int port,
                             std::size_t db) {
  sw::redis::ConnectionOptions options;
  options.host = host;
  options.port = port;
  options.db = static_cast<int>(db);
  options.socket_timeout = std::chrono::milliseconds(200);

  redis_ = std::make_shared<sw::redis::Redis>(options);
}

std::string RedisUrlCache::make_key(const std::string &short_key) const {
  return "url:" + short_key;
}

std::optional<std::string>
RedisUrlCache::get_original_url(const std::string &short_key) {
  auto val = redis_->get(make_key(short_key));
  if (val) {
    return *val;
  }
  return std::nullopt;
}

void RedisUrlCache::put_original_url(const std::string &short_key,
                                     const std::string &original_url,
                                     std::size_t ttl_seconds) {
  redis_->setex(make_key(short_key), static_cast<long long>(ttl_seconds),
                original_url);
}

void RedisUrlCache::remove_original_url(const std::string &short_key) {
  redis_->del(make_key(short_key));
}

} // namespace shortener
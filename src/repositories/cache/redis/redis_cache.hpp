#pragma once

#include "repositories/cache/cache.hpp"
#include <memory>
#include <string>

namespace sw::redis {
class Redis;
}

namespace shortener {

class RedisUrlCache final : public IUrlCache {
private:
  std::shared_ptr<sw::redis::Redis> redis_;

private:
  std::string make_key(const std::string &short_key) const;

public:
  RedisUrlCache(const std::string &host, int port, std::size_t db = 0);

  std::optional<std::string>
  get_original_url(const std::string &short_key) override;

  void put_original_url(const std::string &short_key,
                        const std::string &original_url,
                        std::size_t ttl_seconds) override;

  void remove_original_url(const std::string &short_key) override;
};

} // namespace shortener
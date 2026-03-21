#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

namespace shortener {
class UrlShortener {
public:
  std::string shorten(const std::string &url);

  std::string resolve(const std::string &key);

private:
  std::unordered_map<std::string, std::string> storage_;
  std::mutex mutex_;
};
} // namespace shortener
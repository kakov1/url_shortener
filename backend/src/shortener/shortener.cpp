#include "shortener.hpp"

std::string UrlShortener::shorten(const std::string &url) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::string key = std::to_string(std::hash<std::string>{}(url)).substr(0, 6);
  storage_[key] = url;

  return key;
}

std::string UrlShortener::resolve(const std::string &key) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (storage_.count(key))
    return storage_[key];

  return "";
}
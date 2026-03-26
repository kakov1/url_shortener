#include "in_memory_url_repository.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace shortener {

std::optional<Url> InMemoryUrlRepository::find_by_id(std::int64_t id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto &url : urls_) {
    if (url.id == id) {
      return url;
    }
  }

  return std::nullopt;
}

std::optional<Url>
InMemoryUrlRepository::find_by_short_key(const std::string &short_key) const {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto &url : urls_) {
    if (url.short_key == short_key) {
      return url;
    }
  }

  return std::nullopt;
}

std::optional<Url> InMemoryUrlRepository::find_public_by_original_url(
    const std::string &original_url) const {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto &url : urls_) {
    if (url.original_url == original_url && !url.user_id.has_value()) {
      return url;
    }
  }

  return std::nullopt;
}

std::optional<Url> InMemoryUrlRepository::find_by_original_url_and_user_id(
    const std::string &original_url, std::int64_t user_id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto &url : urls_) {
    if (url.original_url == original_url && url.user_id.has_value() &&
        *url.user_id == user_id) {
      return url;
    }
  }

  return std::nullopt;
}

bool InMemoryUrlRepository::exists_by_short_key(
    const std::string &short_key) const {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto &url : urls_) {
    if (url.short_key == short_key) {
      return true;
    }
  }

  return false;
}

Url InMemoryUrlRepository::create(const std::string &original_url,
                                  const std::string &short_key,
                                  const std::optional<std::int64_t> &user_id) {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto &url : urls_) {
    if (url.short_key == short_key) {
      return url;
    }
  }

  Url url{.id = next_id_++,
          .original_url = original_url,
          .short_key = short_key,
          .user_id = user_id,
          .created_at = make_created_at()};

  urls_.push_back(std::move(url));

  return urls_.back();
}

std::vector<Url>
InMemoryUrlRepository::find_all_by_user_id(std::int64_t user_id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<Url> result;
  result.reserve(urls_.size());

  for (const auto &url : urls_) {
    if (url.user_id.has_value() && *url.user_id == user_id) {
      result.push_back(url);
    }
  }

  return result;
}

std::string InMemoryUrlRepository::make_created_at() const {
  const auto now = std::chrono::system_clock::now();
  const std::time_t now_time = std::chrono::system_clock::to_time_t(now);

  std::tm utc_tm{};
#if defined(_WIN32)
  gmtime_s(&utc_tm, &now_time);
#else
  gmtime_r(&now_time, &utc_tm);
#endif

  std::ostringstream oss;
  oss << std::put_time(&utc_tm, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

} // namespace shortener
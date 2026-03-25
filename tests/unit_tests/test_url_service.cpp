#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "logic/url_service.hpp"
#include "repositories/in_memory/urls/in_memory_url_repository.hpp"
#include "repositories/in_memory/users/in_memory_user_repository.hpp"

namespace shortener {
namespace {

class UrlServiceTest : public ::testing::Test {
protected:
  InMemoryUrlRepository url_repository;
  InMemoryUserRepository user_repository;
  UrlService service{url_repository, user_repository};
};

TEST_F(UrlServiceTest, CreateUserCreatesNewUser) {
  const User user = service.create_user("alice");

  EXPECT_GT(user.id, 0);
  EXPECT_EQ(user.username, "alice");
}

TEST_F(UrlServiceTest, CreateUserReturnsExistingUserForSameUsername) {
  const User first = service.create_user("alice");
  const User second = service.create_user("alice");

  EXPECT_EQ(first.id, second.id);
  EXPECT_EQ(first.username, second.username);
}

TEST_F(UrlServiceTest, CreateUserThrowsOnEmptyUsername) {
  EXPECT_THROW(service.create_user(""), std::invalid_argument);
}

TEST_F(UrlServiceTest, ShortenUrlCreatesShortenedRecordWithoutUser) {
  const Url shortened = service.shorten_url("https://example.com/page");

  EXPECT_GT(shortened.id, 0);
  EXPECT_EQ(shortened.original_url, "https://example.com/page");
  EXPECT_FALSE(shortened.short_key.empty());
  EXPECT_FALSE(shortened.user_id.has_value());
}

TEST_F(UrlServiceTest, ShortenUrlReturnsExistingRecordForSameOriginalUrl) {
  const Url first = service.shorten_url("https://example.com/page");
  const Url second = service.shorten_url("https://example.com/page");

  EXPECT_EQ(first.id, second.id);
  EXPECT_EQ(first.short_key, second.short_key);
}

TEST_F(UrlServiceTest, ShortenUrlThrowsOnEmptyOriginalUrl) {
  EXPECT_THROW(service.shorten_url(""), std::invalid_argument);
}

TEST_F(UrlServiceTest, ShortenUrlWithExistingUserStoresUserId) {
  const User user = service.create_user("alice");

  const Url shortened =
      service.shorten_url("https://example.com/private", user.id);

  ASSERT_TRUE(shortened.user_id.has_value());
  EXPECT_EQ(*shortened.user_id, user.id);
}

TEST_F(UrlServiceTest, ShortenUrlThrowsWhenUserDoesNotExist) {
  EXPECT_THROW(service.shorten_url("https://example.com/page", 9999),
               std::runtime_error);
}

TEST_F(UrlServiceTest, ResolveUrlReturnsOriginalUrlForExistingShortKey) {
  const Url shortened = service.shorten_url("https://example.com/page");

  const auto resolved = service.resolve_url(shortened.short_key);

  ASSERT_TRUE(resolved.has_value());
  EXPECT_EQ(*resolved, "https://example.com/page");
}

TEST_F(UrlServiceTest, ResolveUrlReturnsNulloptForUnknownShortKey) {
  const auto resolved = service.resolve_url("unknown123");

  EXPECT_FALSE(resolved.has_value());
}

TEST_F(UrlServiceTest, ResolveUrlReturnsNulloptForEmptyShortKey) {
  const auto resolved = service.resolve_url("");

  EXPECT_FALSE(resolved.has_value());
}

TEST_F(UrlServiceTest, GetUserUrlsReturnsOnlyUrlsOfRequestedUser) {
  const User alice = service.create_user("alice");
  const User bob = service.create_user("bob");

  const Url alice_url_1 =
      service.shorten_url("https://example.com/a1", alice.id);
  const Url alice_url_2 =
      service.shorten_url("https://example.com/a2", alice.id);
  service.shorten_url("https://example.com/b1", bob.id);
  service.shorten_url("https://example.com/public");

  const auto urls = service.get_user_urls(alice.id);

  ASSERT_EQ(urls.size(), 2);
  EXPECT_EQ(urls[0].user_id, alice.id);
  EXPECT_EQ(urls[1].user_id, alice.id);

  EXPECT_NE(urls[0].short_key, urls[1].short_key);
  EXPECT_TRUE((urls[0].original_url == alice_url_1.original_url &&
               urls[1].original_url == alice_url_2.original_url) ||
              (urls[0].original_url == alice_url_2.original_url &&
               urls[1].original_url == alice_url_1.original_url));
}

TEST_F(UrlServiceTest, GetUserUrlsThrowsWhenUserDoesNotExist) {
  EXPECT_THROW(service.get_user_urls(9999), std::runtime_error);
}

} // namespace
} // namespace shortener
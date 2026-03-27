#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

#include "entities/entities.hpp"
#include "logic/url_service.hpp"
#include "session.hpp"

namespace shortener {

class HttpSession final : public ISession {
private:
  tcp::socket socket_;
  UrlService &url_service_;
  ushort port_;
  beast::flat_buffer buffer_;

public:
  HttpSession(tcp::socket socket, UrlService &url_service, ushort port);

  void handle_session() override;

private:
  using Request = http::request<http::string_body>;
  using Response = http::response<http::string_body>;

  Request read_request();
  void write_response(const Response &response);

  Response handle_request(const Request &request);
  Response handle_post(const Request &request);
  Response handle_post_users(const Request &request);
  Response handle_post_shorten(const Request &request);
  Response handle_get(const Request &request);
  Response handle_get_user_urls(const std::string &path, unsigned version);

  nlohmann::json parse_json_object(const std::string &body) const;
  std::string extract_url_from_json(const nlohmann::json &parsed) const;
  std::optional<std::int64_t>
  extract_user_id_from_json(const nlohmann::json &parsed) const;
  std::string extract_username_from_json(const nlohmann::json &parsed) const;
  std::optional<std::string> extract_optional_username_from_json(
      const nlohmann::json &parsed) const;

  bool is_user_urls_path(const std::string &path) const;
  std::int64_t extract_user_id_from_path(const std::string &path) const;

  Response make_json_response(http::status status, const std::string &body,
                              unsigned version) const;
  Response make_text_response(http::status status, const std::string &body,
                              unsigned version) const;
  Response make_redirect_response(const std::string &location,
                                  unsigned version) const;
};

} // namespace shortener
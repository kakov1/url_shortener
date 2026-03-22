#pragma once

#include "url_service.hpp"
#include "types.hpp"

namespace shortener {

class HttpSession {
private:
  tcp::socket socket_;
  UrlService &url_service_;

  http::response<http::string_body>
  handle_request(const http::request<http::string_body> &request);

  http::response<http::string_body>
  handle_shorten(const http::request<http::string_body> &request);

  http::response<http::string_body>
  handle_redirect(const http::request<http::string_body> &request);

  http::response<http::string_body>
  make_string_response(http::status status, const std::string &body,
                       unsigned version, const std::string &content_type);

  http::response<http::string_body>
  make_redirect_response(const std::string &location, unsigned version);

public:
  HttpSession(tcp::socket socket, UrlService &url_service);

  void handle_session();
};
} // namespace shortener
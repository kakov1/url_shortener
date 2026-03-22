#pragma once

#include "session.hpp"
#include "types.hpp"
#include "url_service.hpp"

namespace shortener {

class HttpSession final : public ISession {
private:
  tcp::socket socket_;
  UrlService &url_service_;
  ushort port_;

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
  HttpSession(tcp::socket socket, UrlService &url_service, ushort port);

  void handle_session() override;
};
} // namespace shortener
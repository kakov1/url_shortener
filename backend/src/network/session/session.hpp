#pragma once

#include "types.hpp"
#include "shortener.hpp"

namespace shortener {
class HttpSession {
private:
  tcp::socket socket_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  UrlShortener &shortener_;

public:
  HttpSession(tcp::socket socket, UrlShortener &shortener);

  void handle();
};
} // namespace shortener
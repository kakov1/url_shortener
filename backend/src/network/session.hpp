#pragma once

#include "../shortener/shortener.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;

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
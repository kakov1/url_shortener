#include "http_session.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/error_code.hpp>

#include <nlohmann/json.hpp>

#include <exception>
#include <iostream>
#include <string>
#include <utility>

namespace shortener {

using json = nlohmann::json;

HttpSession::HttpSession(tcp::socket socket, UrlService &url_service)
    : socket_(std::move(socket)), url_service_(url_service) {}

void HttpSession::handle_session() {
  try {
    beast::flat_buffer buffer;
    http::request<http::string_body> request;

    http::read(socket_, buffer, request);

    auto response = handle_request(request);
    http::write(socket_, response);

    boost::system::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_send, ec);
  } catch (const std::exception &e) {
    std::cerr << "Session error: " << e.what() << std::endl;
  }
}

http::response<http::string_body>
HttpSession::handle_request(const http::request<http::string_body> &request) {

  if (request.method() == http::verb::get && request.target() == "/health") {
    return make_string_response(http::status::ok, "OK", request.version(),
                                "text/plain");
  }

  if (request.method() == http::verb::post && request.target() == "/shorten") {
    return handle_shorten(request);
  }

  if (request.method() == http::verb::get) {
    return handle_redirect(request);
  }

  json error;
  error["error"] = "unsupported request";

  return make_string_response(http::status::bad_request, error.dump(),
                              request.version(), "application/json");
}

http::response<http::string_body>
HttpSession::handle_shorten(const http::request<http::string_body> &request) {
  try {
    json parsed = json::parse(request.body());

    if (!parsed.contains("url") || !parsed["url"].is_string()) {
      json error;
      error["error"] = "field 'url' is required";

      return make_string_response(http::status::bad_request, error.dump(),
                                  request.version(), "application/json");
    }

    std::string original_url = parsed["url"];

    if (original_url.empty()) {
      json error;
      error["error"] = "url cannot be empty";

      return make_string_response(http::status::bad_request, error.dump(),
                                  request.version(), "application/json");
    }

    std::string short_key = url_service_.shorten(original_url);

    json response;
    response["short_key"] = short_key;
    response["short_url"] = "http://localhost:8080/" + short_key;

    return make_string_response(http::status::ok, response.dump(),
                                request.version(), "application/json");

  } catch (const json::parse_error &) {
    json error;
    error["error"] = "invalid json";

    return make_string_response(http::status::bad_request, error.dump(),
                                request.version(), "application/json");
  } catch (const std::exception &e) {
    json error;
    error["error"] = e.what();

    return make_string_response(http::status::internal_server_error,
                                error.dump(), request.version(),
                                "application/json");
  }
}

http::response<http::string_body>
HttpSession::handle_redirect(const http::request<http::string_body> &request) {

  std::string target = std::string(request.target());

  if (target.empty() || target == "/") {
    json error;
    error["error"] = "empty short key";

    return make_string_response(http::status::bad_request, error.dump(),
                                request.version(), "application/json");
  }

  std::string short_key = target.substr(1);

  try {
    std::string original_url = url_service_.restore(short_key);

    if (original_url.empty()) {
      json error;
      error["error"] = "short url not found";

      return make_string_response(http::status::not_found, error.dump(),
                                  request.version(), "application/json");
    }

    return make_redirect_response(original_url, request.version());

  } catch (const std::exception &e) {
    json error;
    error["error"] = e.what();

    return make_string_response(http::status::internal_server_error,
                                error.dump(), request.version(),
                                "application/json");
  }
}

http::response<http::string_body>
HttpSession::make_string_response(http::status status, const std::string &body,
                                  unsigned version,
                                  const std::string &content_type) {
  http::response<http::string_body> response{status, version};
  response.set(http::field::server, "url-shortener");
  response.set(http::field::content_type, content_type);
  response.keep_alive(false);
  response.body() = body;
  response.prepare_payload();
  return response;
}

http::response<http::string_body>
HttpSession::make_redirect_response(const std::string &location,
                                    unsigned version) {
  http::response<http::string_body> response{http::status::found, version};
  response.set(http::field::server, "url-shortener");
  response.set(http::field::location, location);
  response.keep_alive(false);
  response.prepare_payload();
  return response;
}

} // namespace shortener
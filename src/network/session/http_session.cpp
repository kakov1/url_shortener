#include "http_session.hpp"

#include <cstdint>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/asio/write.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

namespace shortener {

namespace {
constexpr std::size_t kMaxRequestSize = 8 * 1024;
}

HttpSession::HttpSession(tcp::socket socket, UrlService &url_service,
                         unsigned short port)
    : socket_(std::move(socket)), url_service_(url_service), port_(port),
      buffer_() {}

void HttpSession::handle_session() {
  try {
    const Request request = read_request();
    const Response response = handle_request(request);
    write_response(response);
  } catch (const beast::system_error &ex) {
    if (ex.code() != http::error::end_of_stream) {
      try {
        const Response response = make_text_response(
            http::status::internal_server_error, "Internal Server Error", 11);
        write_response(response);
      } catch (...) {
      }
    }
  } catch (const std::exception &) {
    try {
      const Response response = make_text_response(
          http::status::internal_server_error, "Internal Server Error", 11);
      write_response(response);
    } catch (...) {
    }
  }

  boost::system::error_code ec;
  socket_.shutdown(tcp::socket::shutdown_both, ec);
  ec.clear();
  socket_.close(ec);
}

HttpSession::Request HttpSession::read_request() {
  Request request;
  http::read(socket_, buffer_, request);

  if (request.body().size() > kMaxRequestSize) {
    throw std::runtime_error("request too large");
  }

  return request;
}

void HttpSession::write_response(const Response &response) {
  http::write(socket_, response);
}

HttpSession::Response HttpSession::handle_request(const Request &request) {
  switch (request.method()) {
  case http::verb::post:
    return handle_post(request);
  case http::verb::get:
    return handle_get(request);
  default:
    return make_text_response(http::status::method_not_allowed,
                              "Method Not Allowed", request.version());
  }
}

HttpSession::Response HttpSession::handle_post(const Request &request) {
  const std::string path = std::string(request.target());

  if (path == "/users") {
    return handle_post_users(request);
  }

  if (path == "/shorten") {
    return handle_post_shorten(request);
  }

  return make_text_response(http::status::not_found, "Not Found",
                            request.version());
}

HttpSession::Response HttpSession::handle_post_users(const Request &request) {
  try {
    const auto parsed = parse_json_object(request.body());
    const std::string username = extract_username_from_json(parsed);

    const User user = url_service_.create_user(username);

    nlohmann::json response_body = {
        {"id", user.id},
        {"username", user.username},
    };

    return make_json_response(http::status::created, response_body.dump(),
                              request.version());
  } catch (const std::invalid_argument &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(http::status::bad_request, error_body.dump(),
                              request.version());
  } catch (const nlohmann::json::exception &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(http::status::bad_request, error_body.dump(),
                              request.version());
  } catch (const std::exception &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(http::status::internal_server_error,
                              error_body.dump(), request.version());
  }
}

HttpSession::Response HttpSession::handle_post_shorten(const Request &request) {
  try {
    const auto parsed = parse_json_object(request.body());

    const std::string original_url = extract_url_from_json(parsed);
    std::optional<std::int64_t> user_id = extract_user_id_from_json(parsed);
    const std::optional<std::string> username =
        extract_optional_username_from_json(parsed);

    if (!user_id.has_value() && username.has_value()) {
      const User user = url_service_.get_or_create_user(*username);
      user_id = user.id;
    }

    const Url shortened = url_service_.shorten_url(original_url, user_id);

    nlohmann::json response_body = {
        {"id", shortened.id},
        {"original_url", shortened.original_url},
        {"short_key", shortened.short_key},
        {"short_url", "http://localhost:" + std::to_string(port_) + "/" +
                          shortened.short_key},
        {"created_at", shortened.created_at},
    };

    if (shortened.user_id.has_value()) {
      response_body["user_id"] = *shortened.user_id;
    } else {
      response_body["user_id"] = nullptr;
    }

    return make_json_response(http::status::created, response_body.dump(),
                              request.version());
  } catch (const std::invalid_argument &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(http::status::bad_request, error_body.dump(),
                              request.version());
  } catch (const nlohmann::json::exception &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(http::status::bad_request, error_body.dump(),
                              request.version());
  } catch (const std::runtime_error &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    if (std::string(ex.what()) == "user not found") {
      return make_json_response(http::status::not_found, error_body.dump(),
                                request.version());
    }

    return make_json_response(http::status::internal_server_error,
                              error_body.dump(), request.version());
  } catch (const std::exception &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(http::status::internal_server_error,
                              error_body.dump(), request.version());
  }
}

HttpSession::Response HttpSession::handle_get(const Request &request) {
  const std::string path = std::string(request.target());

  if (path == "/health") {
    nlohmann::json response_body = {{"status", "ok"}};

    return make_json_response(http::status::ok, response_body.dump(),
                              request.version());
  }

  if (is_user_urls_path(path)) {
    return handle_get_user_urls(path, request.version());
  }

  if (path.empty() || path == "/") {
    return make_text_response(http::status::not_found, "Not Found",
                              request.version());
  }

  const std::string short_key = path.substr(1);
  const auto original_url = url_service_.resolve_url(short_key);

  if (!original_url.has_value()) {
    return make_text_response(http::status::not_found, "Short URL not found",
                              request.version());
  }

  return make_redirect_response(*original_url, request.version());
}

HttpSession::Response HttpSession::handle_get_user_urls(const std::string &path,
                                                        unsigned version) {
  try {
    const std::int64_t user_id = extract_user_id_from_path(path);
    const std::vector<Url> urls = url_service_.get_user_urls(user_id);

    nlohmann::json urls_json = nlohmann::json::array();

    for (const auto &url : urls) {
      nlohmann::json item = {
          {"id", url.id},
          {"original_url", url.original_url},
          {"short_key", url.short_key},
          {"short_url",
           "http://localhost:" + std::to_string(port_) + "/" + url.short_key},
          {"created_at", url.created_at},
      };

      if (url.user_id.has_value()) {
        item["user_id"] = *url.user_id;
      } else {
        item["user_id"] = nullptr;
      }

      urls_json.push_back(item);
    }

    nlohmann::json response_body = {
        {"user_id", user_id},
        {"urls", urls_json},
    };

    return make_json_response(http::status::ok, response_body.dump(), version);
  } catch (const std::invalid_argument &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(http::status::bad_request, error_body.dump(),
                              version);
  } catch (const std::runtime_error &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(http::status::not_found, error_body.dump(),
                              version);
  } catch (const std::exception &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(http::status::internal_server_error,
                              error_body.dump(), version);
  }
}

nlohmann::json HttpSession::parse_json_object(const std::string &body) const {
  const auto parsed = nlohmann::json::parse(body);

  if (!parsed.is_object()) {
    throw std::invalid_argument("request body must be a JSON object");
  }

  return parsed;
}

std::string
HttpSession::extract_url_from_json(const nlohmann::json &parsed) const {
  if (!parsed.contains("url")) {
    throw std::invalid_argument("field 'url' is required");
  }

  if (!parsed["url"].is_string()) {
    throw std::invalid_argument("field 'url' must be a string");
  }

  const std::string url = parsed["url"].get<std::string>();
  if (url.empty()) {
    throw std::invalid_argument("field 'url' cannot be empty");
  }

  return url;
}

std::optional<std::int64_t>
HttpSession::extract_user_id_from_json(const nlohmann::json &parsed) const {
  if (!parsed.contains("user_id") || parsed["user_id"].is_null()) {
    return std::nullopt;
  }

  if (parsed["user_id"].is_number_integer()) {
    const std::int64_t user_id = parsed["user_id"].get<std::int64_t>();
    if (user_id <= 0) {
      throw std::invalid_argument("field 'user_id' must be positive");
    }
    return user_id;
  }

  if (parsed["user_id"].is_string()) {
    const std::string raw_user_id = parsed["user_id"].get<std::string>();
    if (raw_user_id.empty()) {
      throw std::invalid_argument("field 'user_id' cannot be empty");
    }

    std::size_t processed = 0;
    const std::int64_t user_id = std::stoll(raw_user_id, &processed);

    if (processed != raw_user_id.size()) {
      throw std::invalid_argument("field 'user_id' must be an integer");
    }

    if (user_id <= 0) {
      throw std::invalid_argument("field 'user_id' must be positive");
    }

    return user_id;
  }

  throw std::invalid_argument("field 'user_id' must be integer or string");
}

std::string
HttpSession::extract_username_from_json(const nlohmann::json &parsed) const {
  if (!parsed.contains("username")) {
    throw std::invalid_argument("field 'username' is required");
  }

  if (!parsed["username"].is_string()) {
    throw std::invalid_argument("field 'username' must be a string");
  }

  const std::string username = parsed["username"].get<std::string>();
  if (username.empty()) {
    throw std::invalid_argument("field 'username' cannot be empty");
  }

  return username;
}

std::optional<std::string> HttpSession::extract_optional_username_from_json(
    const nlohmann::json &parsed) const {
  if (!parsed.contains("username") || parsed["username"].is_null()) {
    return std::nullopt;
  }

  if (!parsed["username"].is_string()) {
    throw std::invalid_argument("field 'username' must be a string");
  }

  const std::string username = parsed["username"].get<std::string>();
  if (username.empty()) {
    throw std::invalid_argument("field 'username' cannot be empty");
  }

  return username;
}

bool HttpSession::is_user_urls_path(const std::string &path) const {
  if (path.rfind("/users/", 0) != 0) {
    return false;
  }

  const std::string suffix = "/urls";
  if (path.size() <= 7 + suffix.size()) {
    return false;
  }

  const std::size_t suffix_pos = path.size() - suffix.size();
  return path.substr(suffix_pos) == suffix;
}

std::int64_t
HttpSession::extract_user_id_from_path(const std::string &path) const {
  if (!is_user_urls_path(path)) {
    throw std::invalid_argument("invalid users urls path");
  }

  const std::size_t prefix_len = std::string("/users/").size();
  const std::size_t suffix_pos = path.rfind("/urls");

  const std::string raw_id = path.substr(prefix_len, suffix_pos - prefix_len);
  if (raw_id.empty()) {
    throw std::invalid_argument("user id is required");
  }

  std::size_t processed = 0;
  const std::int64_t user_id = std::stoll(raw_id, &processed);

  if (processed != raw_id.size()) {
    throw std::invalid_argument("user id must be an integer");
  }

  if (user_id <= 0) {
    throw std::invalid_argument("user id must be positive");
  }

  return user_id;
}

HttpSession::Response HttpSession::make_json_response(http::status status,
                                                      const std::string &body,
                                                      unsigned version) const {
  Response response{status, version};
  response.set(http::field::content_type, "application/json");
  response.set(http::field::connection, "close");
  response.keep_alive(false);
  response.body() = body;
  response.prepare_payload();
  return response;
}

HttpSession::Response HttpSession::make_text_response(http::status status,
                                                      const std::string &body,
                                                      unsigned version) const {
  Response response{status, version};
  response.set(http::field::content_type, "text/plain");
  response.set(http::field::connection, "close");
  response.keep_alive(false);
  response.body() = body;
  response.prepare_payload();
  return response;
}

HttpSession::Response
HttpSession::make_redirect_response(const std::string &location,
                                    unsigned version) const {
  Response response{http::status::found, version};
  response.set(http::field::location, location);
  response.set(http::field::connection, "close");
  response.keep_alive(false);
  response.body() = "";
  response.prepare_payload();
  return response;
}

} // namespace shortener

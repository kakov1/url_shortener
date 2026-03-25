#include "http_session.hpp"

#include <cstdint>
#include <exception>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <nlohmann/json.hpp>

namespace shortener {

namespace {
constexpr std::size_t kMaxRequestSize = 8192;
}

HttpSession::HttpSession(boost::asio::ip::tcp::socket socket,
                         UrlService &url_service, unsigned short port)
    : socket_(std::move(socket)), url_service_(url_service), port_(port) {}

void HttpSession::handle_session() {
  try {
    const std::string request = read_request();
    const std::string response = handle_request(request);
    write_response(response);
  } catch (const std::exception &) {
    const std::string response = make_text_response(
        500, "Internal Server Error", "Internal Server Error");
    write_response(response);
  }

  boost::system::error_code ec;
  socket_.shutdown(tcp::socket::shutdown_both, ec);
  ec.clear();
  socket_.close(ec);
}

std::string HttpSession::read_request() {
  boost::asio::streambuf buffer;
  boost::asio::read_until(socket_, buffer, "\r\n\r\n");

  std::istream stream(&buffer);
  std::string request_headers((std::istreambuf_iterator<char>(stream)),
                              std::istreambuf_iterator<char>());

  const std::size_t header_end = request_headers.find("\r\n\r\n");
  if (header_end == std::string::npos) {
    throw std::runtime_error("invalid HTTP request");
  }

  std::size_t content_length = 0;
  {
    const std::string header_name = "Content-Length:";
    const std::size_t pos = request_headers.find(header_name);

    if (pos != std::string::npos) {
      const std::size_t value_start = pos + header_name.size();
      const std::size_t line_end = request_headers.find("\r\n", value_start);
      const std::string raw_value =
          request_headers.substr(value_start, line_end - value_start);

      std::stringstream ss(raw_value);
      ss >> content_length;
    }
  }

  std::string full_request = request_headers;
  const std::size_t current_body_size = full_request.size() - (header_end + 4);

  if (content_length > current_body_size) {
    const std::size_t remaining = content_length - current_body_size;

    if (full_request.size() + remaining > kMaxRequestSize) {
      throw std::runtime_error("request too large");
    }

    std::string rest(remaining, '\0');
    boost::asio::read(socket_, boost::asio::buffer(rest.data(), remaining));
    full_request += rest;
  }

  return full_request;
}

void HttpSession::write_response(const std::string &response) {
  boost::asio::write(socket_, boost::asio::buffer(response));
}

std::string HttpSession::handle_request(const std::string &request) {
  const std::string method = extract_method(request);

  if (method == "POST") {
    return handle_post(request);
  }

  if (method == "GET") {
    return handle_get(request);
  }

  return make_text_response(405, "Method Not Allowed", "Method Not Allowed");
}

std::string HttpSession::handle_post(const std::string &request) {
  const std::string path = extract_path(request);

  if (path == "/users") {
    return handle_post_users(request);
  }

  if (path == "/shorten") {
    return handle_post_shorten(request);
  }

  return make_text_response(404, "Not Found", "Not Found");
}

std::string HttpSession::handle_post_users(const std::string &request) {
  try {
    const std::string body = extract_body(request);
    const std::string username = extract_username_from_json(body);

    const User user = url_service_.create_user(username);

    nlohmann::json response_body = {{"id", user.id},
                                    {"username", user.username}};

    return make_json_response(201, "Created", response_body.dump());
  } catch (const std::invalid_argument &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(400, "Bad Request", error_body.dump());
  } catch (const nlohmann::json::exception &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(400, "Bad Request", error_body.dump());
  } catch (const std::exception &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(500, "Internal Server Error", error_body.dump());
  }
}

std::string HttpSession::handle_post_shorten(const std::string &request) {
  try {
    const std::string body = extract_body(request);
    const std::string original_url = extract_url_from_json(body);
    const std::string raw_user_id = extract_user_id_from_json(body);

    std::optional<std::int64_t> user_id = std::nullopt;
    if (!raw_user_id.empty()) {
      user_id = std::stoll(raw_user_id);
    }

    const Url shortened = url_service_.shorten_url(original_url, user_id);

    nlohmann::json response_body = {
        {"id", shortened.id},
        {"original_url", shortened.original_url},
        {"short_key", shortened.short_key},
        {"short_url", "http://localhost:" + std::to_string(port_) + "/" +
                          shortened.short_key},
        {"created_at", shortened.created_at}};

    if (shortened.user_id.has_value()) {
      response_body["user_id"] = *shortened.user_id;
    } else {
      response_body["user_id"] = nullptr;
    }

    return make_json_response(201, "Created", response_body.dump());
  } catch (const std::invalid_argument &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(400, "Bad Request", error_body.dump());
  } catch (const nlohmann::json::exception &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(400, "Bad Request", error_body.dump());
  } catch (const std::exception &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(500, "Internal Server Error", error_body.dump());
  }
}

std::string HttpSession::handle_get(const std::string &request) {
  const std::string path = extract_path(request);

  if (path == "/health") {
    nlohmann::json response_body = {{"status", "ok"}};

    return make_json_response(200, "OK", response_body.dump());
  }

  if (is_user_urls_path(path)) {
    return handle_get_user_urls(path);
  }

  if (path.empty() || path == "/") {
    return make_text_response(404, "Not Found", "Not Found");
  }

  const std::string short_key = path.substr(1);

  const auto original_url = url_service_.resolve_url(short_key);
  if (!original_url.has_value()) {
    return make_text_response(404, "Not Found", "Short URL not found");
  }

  return make_redirect_response(*original_url);
}

std::string HttpSession::handle_get_user_urls(const std::string &path) {
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
          {"created_at", url.created_at}};

      if (url.user_id.has_value()) {
        item["user_id"] = *url.user_id;
      } else {
        item["user_id"] = nullptr;
      }

      urls_json.push_back(item);
    }

    nlohmann::json response_body = {{"user_id", user_id}, {"urls", urls_json}};

    return make_json_response(200, "OK", response_body.dump());
  } catch (const std::invalid_argument &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(400, "Bad Request", error_body.dump());
  } catch (const std::runtime_error &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(404, "Not Found", error_body.dump());
  } catch (const std::exception &ex) {
    nlohmann::json error_body = {{"error", ex.what()}};

    return make_json_response(500, "Internal Server Error", error_body.dump());
  }
}

std::string HttpSession::extract_method(const std::string &request) const {
  const std::size_t first_space = request.find(' ');
  if (first_space == std::string::npos) {
    throw std::runtime_error("invalid HTTP request line");
  }

  return request.substr(0, first_space);
}

std::string HttpSession::extract_path(const std::string &request) const {
  const std::size_t first_space = request.find(' ');
  if (first_space == std::string::npos) {
    throw std::runtime_error("invalid HTTP request line");
  }

  const std::size_t second_space = request.find(' ', first_space + 1);
  if (second_space == std::string::npos) {
    throw std::runtime_error("invalid HTTP request line");
  }

  return request.substr(first_space + 1, second_space - first_space - 1);
}

std::string HttpSession::extract_body(const std::string &request) const {
  const std::size_t body_pos = request.find("\r\n\r\n");
  if (body_pos == std::string::npos) {
    return {};
  }

  return request.substr(body_pos + 4);
}

std::string HttpSession::extract_url_from_json(const std::string &body) const {
  const auto parsed = nlohmann::json::parse(body);

  if (!parsed.is_object()) {
    throw std::invalid_argument("request body must be a JSON object");
  }

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

std::string
HttpSession::extract_user_id_from_json(const std::string &body) const {
  const auto parsed = nlohmann::json::parse(body);

  if (!parsed.is_object()) {
    return {};
  }

  if (!parsed.contains("user_id") || parsed["user_id"].is_null()) {
    return {};
  }

  if (parsed["user_id"].is_number_integer()) {
    return std::to_string(parsed["user_id"].get<std::int64_t>());
  }

  if (parsed["user_id"].is_string()) {
    return parsed["user_id"].get<std::string>();
  }

  throw std::invalid_argument("field 'user_id' must be integer or string");
}

std::string
HttpSession::extract_username_from_json(const std::string &body) const {
  const auto parsed = nlohmann::json::parse(body);

  if (!parsed.is_object()) {
    throw std::invalid_argument("request body must be a JSON object");
  }

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

std::string HttpSession::make_json_response(int status_code,
                                            const std::string &status_text,
                                            const std::string &body) const {
  std::ostringstream response;
  response << "HTTP/1.1 " << status_code << ' ' << status_text << "\r\n";
  response << "Content-Type: application/json\r\n";
  response << "Content-Length: " << body.size() << "\r\n";
  response << "Connection: close\r\n";
  response << "\r\n";
  response << body;
  return response.str();
}

std::string
HttpSession::make_redirect_response(const std::string &location) const {
  std::ostringstream response;
  response << "HTTP/1.1 302 Found\r\n";
  response << "Location: " << location << "\r\n";
  response << "Content-Length: 0\r\n";
  response << "Connection: close\r\n";
  response << "\r\n";
  return response.str();
}

std::string HttpSession::make_text_response(int status_code,
                                            const std::string &status_text,
                                            const std::string &body) const {
  std::ostringstream response;
  response << "HTTP/1.1 " << status_code << ' ' << status_text << "\r\n";
  response << "Content-Type: text/plain\r\n";
  response << "Content-Length: " << body.size() << "\r\n";
  response << "Connection: close\r\n";
  response << "\r\n";
  response << body;
  return response.str();
}

} // namespace shortener

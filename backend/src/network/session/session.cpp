#include "session.hpp"

namespace shortener {

HttpSession::HttpSession(tcp::socket socket, UrlShortener &shortener)
    : socket_(std::move(socket)), shortener_(shortener) {}

void HttpSession::handle() {
  boost::beast::flat_buffer buffer;

  http::request<http::string_body> req;
  http::read(socket_, buffer, req);

  http::response<http::string_body> res;

  if (req.method() == http::verb::post && req.target() == "/shorten") {
    std::string url = req.body();
    std::string key = shortener_.shorten(url);

    res.result(http::status::ok);
    res.body() = key;

  } else if (req.method() == http::verb::get) {
    std::string target = std::string(req.target());

    if (target.size() > 1) {
      std::string key = target.substr(1);
      std::string url = shortener_.resolve(key);

      if (!url.empty()) {
        res.result(http::status::found);
        res.set(http::field::location, url);
      } else {
        res.result(http::status::not_found);
        res.body() = "Not found";
      }
    } else {
      res.result(http::status::bad_request);
      res.body() = "Empty key";
    }

  } else {
    res.result(http::status::bad_request);
    res.body() = "Invalid request";
  }

  res.version(req.version());
  res.set(http::field::server, "CustomServer");
  res.set(http::field::content_type, "text/plain");
  res.keep_alive(false);

  res.prepare_payload();

  http::write(socket_, res);

  boost::system::error_code ec;
  socket_.shutdown(tcp::socket::shutdown_send, ec);
}
} // namespace shortener
#pragma once

#include <string>

#include <boost/asio/ip/tcp.hpp>

#include "entities/entities.hpp"
#include "logic/url_service.hpp"
#include "session.hpp"

namespace shortener {

class HttpSession final : public ISession {
private:
  tcp::socket socket_;
  UrlService &url_service_;
  ushort port_;

public:
  HttpSession(tcp::socket socket, UrlService &url_service, unsigned short port);

  void handle_session() override;

private:
  std::string read_request();
  void write_response(const std::string &response);

  std::string handle_request(const std::string &request);
  std::string handle_post(const std::string &request);
  std::string handle_post_users(const std::string &request);
  std::string handle_post_shorten(const std::string &request);
  std::string handle_get(const std::string &request);
  std::string handle_get_user_urls(const std::string &path);

  std::string extract_method(const std::string &request) const;
  std::string extract_path(const std::string &request) const;
  std::string extract_body(const std::string &request) const;

  std::string extract_url_from_json(const std::string &body) const;
  std::string extract_user_id_from_json(const std::string &body) const;
  std::string extract_username_from_json(const std::string &body) const;

  bool is_user_urls_path(const std::string &path) const;
  std::int64_t extract_user_id_from_path(const std::string &path) const;

  std::string make_json_response(int status_code,
                                 const std::string &status_text,
                                 const std::string &body) const;

  std::string make_redirect_response(const std::string &location) const;
  std::string make_text_response(int status_code,
                                 const std::string &status_text,
                                 const std::string &body) const;
};

} // namespace shortener::network
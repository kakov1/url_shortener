#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <boost/beast.hpp>
#include <boost/beast/http.hpp>

namespace shortener {

namespace beast = boost::beast;
namespace http = beast::http;

using io_context = boost::asio::io_context;
using tcp = boost::asio::ip::tcp;

using ushort = unsigned short;

class User final {
public:
  std::int64_t id;
  std::string username;
};

class Url final {
public:
  std::int64_t id;
  std::string original_url;
  std::string short_key;
  std::optional<std::int64_t> user_id;
  std::string created_at;
};

} // namespace shortener
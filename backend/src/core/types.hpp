#pragma once

#include <boost/beast.hpp>
#include <boost/beast/http.hpp>

namespace shortener {

namespace beast = boost::beast;
namespace http = beast::http;

using io_context = boost::asio::io_context;
using tcp = boost::asio::ip::tcp;

using ushort = unsigned short;


} // namespace shortener
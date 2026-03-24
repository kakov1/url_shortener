#pragma once

#include <string>

namespace shortener {

using ushort = unsigned short;

struct DbConfig {
  std::string host = "127.0.0.1";
  int port = 5432;
  std::string name = "url_shortener";
  std::string user = "postgres";
  std::string password = "postgres";
};

struct AppConfig {
  ushort port = 8080;
  ushort num_threads = 4;
  DbConfig db;
};

AppConfig parse_config(int argc, char *argv[]);

std::string make_connection_string(const DbConfig& db_config);

} // namespace shortener
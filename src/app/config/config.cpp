#include "config.hpp"

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace shortener {
namespace {

void print_help_and_exit(const po::options_description& desc) {
  std::cout << desc << '\n';
  std::exit(0);
}

void validate_config(const AppConfig& config) {
  if (config.port == 0) {
    throw std::runtime_error("server port must be greater than 0");
  }

  if (config.num_threads == 0) {
    throw std::runtime_error("number of worker threads must be greater than 0");
  }

  if (config.db.port <= 0 || config.db.port > 65535) {
    throw std::runtime_error("database port must be in range [1, 65535]");
  }

  if (config.db.name.empty()) {
    throw std::runtime_error("database name must not be empty");
  }

  if (config.db.user.empty()) {
    throw std::runtime_error("database user must not be empty");
  }
}

void override_from_env(AppConfig& config) {
  if (const char* value = std::getenv("DB_HOST")) {
    config.db.host = value;
  }

  if (const char* value = std::getenv("DB_PORT")) {
    config.db.port = std::stoi(value);
  }

  if (const char* value = std::getenv("DB_NAME")) {
    config.db.name = value;
  }

  if (const char* value = std::getenv("DB_USER")) {
    config.db.user = value;
  }

  if (const char* value = std::getenv("DB_PASSWORD")) {
    config.db.password = value;
  }

  if (const char* value = std::getenv("APP_PORT")) {
    config.port = static_cast<ushort>(std::stoi(value));
  }

  if (const char* value = std::getenv("APP_THREADS")) {
    config.num_threads = static_cast<ushort>(std::stoi(value));
  }
}

po::options_description make_options_description(AppConfig& config) {
  po::options_description desc("Allowed options");

  desc.add_options()
      ("help,h", "show help message")
      ("port,p",
       po::value<ushort>(&config.port)->default_value(config.port),
       "server port")
      ("threads,t",
       po::value<ushort>(&config.num_threads)->default_value(config.num_threads),
       "number of worker threads")
      ("db-host",
       po::value<std::string>(&config.db.host)->default_value(config.db.host),
       "postgres host")
      ("db-port",
       po::value<int>(&config.db.port)->default_value(config.db.port),
       "postgres port")
      ("db-name",
       po::value<std::string>(&config.db.name)->default_value(config.db.name),
       "postgres database name")
      ("db-user",
       po::value<std::string>(&config.db.user)->default_value(config.db.user),
       "postgres user")
      ("db-password",
       po::value<std::string>(&config.db.password)->default_value(config.db.password),
       "postgres password");

  return desc;
}

}  // namespace

AppConfig parse_config(int argc, char* argv[]) {
  AppConfig config;

  override_from_env(config);

  po::options_description desc = make_options_description(config);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (!vm["help"].empty()) {
    print_help_and_exit(desc);
  }

  po::notify(vm);

  validate_config(config);
  return config;
}

std::string make_connection_string(const DbConfig& db) {
  std::ostringstream ss;

  ss << "host=" << db.host
     << " port=" << db.port
     << " dbname=" << db.name
     << " user=" << db.user
     << " password=" << db.password;

  return ss.str();
}

}  // namespace shortener
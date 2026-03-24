#include <exception>
#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "logic/url_service.hpp"
#include "network/server/server.hpp"
#include "repositories/postgres/postgres_connection_pool.hpp"
#include "repositories/postgres/urls/postgres_url_repository.hpp"
#include "repositories/postgres/users/postgres_user_repository.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
  try {
    shortener::ushort port = 8080;
    shortener::ushort num_threads = 4;

    std::string db_host = "127.0.0.1";
    int db_port = 5432;
    std::string db_name = "url_shortener";
    std::string db_user = "postgres";
    std::string db_password = "postgres";

    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "show help message")(
        "port,p", po::value<unsigned short>(&port)->default_value(8080),
        "server port")(
        "threads,t", po::value<unsigned short>(&num_threads)->default_value(4),
        "number of worker threads")(
        "db-host", po::value<std::string>(&db_host)->default_value("127.0.0.1"),
        "postgres host")("db-port",
                         po::value<int>(&db_port)->default_value(5432),
                         "postgres port")(
        "db-name",
        po::value<std::string>(&db_name)->default_value("url_shortener"),
        "postgres database name")(
        "db-user", po::value<std::string>(&db_user)->default_value("postgres"),
        "postgres user")(
        "db-password",
        po::value<std::string>(&db_password)->default_value("postgres"),
        "postgres password");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (!vm["help"].empty()) {
      std::cout << desc << '\n';
      return 0;
    }

    if (num_threads <= 0) {
      throw std::invalid_argument("threads must be greater than zero");
    }

    const std::string connection_string =
        "host=" + db_host + " port=" + std::to_string(db_port) +
        " dbname=" + db_name + " user=" + db_user + " password=" + db_password;

    shortener::PostgresConnectionPool connection_pool(
        connection_string, static_cast<std::size_t>(num_threads));

    shortener::PostgresUrlRepository url_repository(connection_pool);
    shortener::PostgresUserRepository user_repository(connection_pool);
    
    shortener::UrlService url_service(url_repository, user_repository);

    shortener::HttpServer server(port, num_threads, url_service);

    std::cout << "Server started on port " << port << std::endl;
    std::cout << "Worker threads: " << num_threads << std::endl;
    std::cout << "PostgreSQL: " << db_host << ":" << db_port
              << " db=" << db_name << std::endl;

    server.run();
  } catch (const std::exception &ex) {
    std::cerr << "Fatal error: " << ex.what() << '\n';
    return 1;
  }

  return 0;
}
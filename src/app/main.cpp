#include <exception>
#include <iostream>
#include <memory>

#include "repositories/cache/redis/redis_cache.hpp"
#include "config/config.hpp"
#include "logic/url_service.hpp"
#include "network/server/server.hpp"
#include "repositories/postgres/postgres_connection_pool.hpp"
#include "repositories/postgres/urls/postgres_url_repository.hpp"
#include "repositories/postgres/users/postgres_user_repository.hpp"

int main(int argc, char *argv[]) {
  try {
    const shortener::AppConfig config = shortener::parse_config(argc, argv);
    const std::string connection_string =
        shortener::make_connection_string(config.db);

    shortener::PostgresConnectionPool connection_pool(connection_string);
    shortener::PostgresUrlRepository url_repository(connection_pool);
    shortener::PostgresUserRepository user_repository(connection_pool);
    shortener::RedisUrlCache url_cache("redis", 6379, 0);

    shortener::UrlService url_service(url_repository, user_repository,
                                      url_cache, 3600);

    shortener::HttpServer server(config.port, config.num_threads, url_service);
    server.run();

    return 0;
  } catch (const std::exception &ex) {
    std::cerr << "Application error: " << ex.what() << '\n';
    return 1;
  }
}

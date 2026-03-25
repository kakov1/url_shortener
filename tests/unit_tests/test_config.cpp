#include <gtest/gtest.h>

#include <string>

#include "app/config/config.hpp"

namespace shortener {
namespace {

TEST(ConfigTest, MakeConnectionStringBuildsExpectedString) {
  DbConfig db;
  db.host = "127.0.0.1";
  db.port = 5432;
  db.name = "url_shortener";
  db.user = "postgres";
  db.password = "secret";

  const std::string connection_string = make_connection_string(db);

  EXPECT_EQ(connection_string,
            "host=127.0.0.1 port=5432 dbname=url_shortener user=postgres "
            "password=secret");
}

TEST(ConfigTest, MakeConnectionStringSupportsDifferentValues) {
  DbConfig db;
  db.host = "db";
  db.port = 6543;
  db.name = "prod_db";
  db.user = "app_user";
  db.password = "app_pass";

  const std::string connection_string = make_connection_string(db);

  EXPECT_EQ(connection_string, "host=db port=6543 dbname=prod_db user=app_user "
                               "password=app_pass");
}

} // namespace
} // namespace shortener
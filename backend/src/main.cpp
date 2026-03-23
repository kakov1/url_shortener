#include "network/server/server.hpp"
#include "logic/in_memory_url_repository.hpp"

#include <boost/program_options.hpp>

#include <exception>
#include <iostream>

// TODO: cmake, refactoring, postgresql, redis

namespace options = boost::program_options;

int main(int argc, char *argv[]) {
  try {
    shortener::ushort port = 8080;
    shortener::ushort threads = 4;

    options::options_description desc("Available options");
    desc.add_options()("help,h", "show help message")(
        "port,p", options::value<shortener::ushort>(&port)->default_value(8080),
        "server port")("threads,t",
                       options::value<shortener::ushort>(&threads)->default_value(4),
                       "worker threads count");

    options::variables_map vm;
    options::store(options::parse_command_line(argc, argv, desc), vm);
    options::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 0;
    }

    if (threads == 0) {
      std::cerr << "threads must be greater than 0" << std::endl;
      return 1;
    }

    shortener::io_context io_context;
    shortener::InMemoryUrlRepository url_repository;
    shortener::UrlService url_service{url_repository};
    shortener::HttpServer server(io_context, port, threads, url_service);

    std::cout << "Starting server on port " << port << " with " << threads
              << " worker threads" << std::endl;

    server.run();
  } catch (const options::error &e) {
    std::cerr << "Argument parsing error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Fatal error: unknown exception" << std::endl;
    return 1;
  }

  return 0;
}

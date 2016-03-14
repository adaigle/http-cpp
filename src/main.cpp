#include "http_server.h"

#include "logger.hpp"

int main(int argc, char* argv[])
{
    logger::log_level(logger::type::trace);
    logger::info() << "Starting the server..." << logger::endl;

    http_server server;
    server.connect(8081, "hello_world", "my_localhost_website");
    server.connect(8082, "8082", "my_second_website");
    server.connect(8083, "other_website", "my_second_website");
    server.run();

    return 0;
}

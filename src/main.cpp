#include "http_server.h"

#include "logger.h"

int main(int argc, char* argv[])
{
    logger::log(logger::type::server)->set_level(spdlog::level::trace);
    logger::log(logger::type::worker)->set_level(spdlog::level::debug);
    logger::log(logger::type::server)->info() << "Starting the server...";

    http_server server;
    void connect(const std::string& website_path, const std::string& host, uint16_t port = 80, const std::string& website_name = "");

    server.connect("hello_world", "localhost", 8081);
    server.connect("example/rest_webservice.dll", "localhost", 8082);
    server.run();

    return 0;
}

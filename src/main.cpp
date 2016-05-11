#include "http_server.h"

#include <spdlog/spdlog.h>

int main(int argc, char* argv[])
{
    auto server_log = spdlog::stdout_logger_mt("server2", true);
    server_log->set_level(spdlog::level::trace);
    server_log->info() << "Starting the server...";

    std::cout << std::endl;
    server_log->trace() << "color test";
    server_log->debug() << "color test";
    server_log->info() << "color test";
    server_log->notice() << "color test";
    server_log->warn() << "color test";
    server_log->error() << "color test";
    server_log->critical() << "color test";
    server_log->alert() << "color test";
    server_log->emerg() << "color test";
    std::cout << std::endl;

    http_server server;
    void connect(const std::string& website_path, const std::string& host, uint16_t port = 80, const std::string& website_name = "");

    server.connect("hello_world", "localhost", 8081);
    server.connect("example/rest_webservice.dll", "localhost", 8082);
    server.run();

    return 0;
}

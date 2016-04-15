#include "logger.h"

std::map<logger::type, logger::logger_t> logger::console = {
    {logger::type::server, spdlog::stdout_logger_mt("server", true)},
    {logger::type::worker, spdlog::stdout_logger_mt("worker", true)}
};

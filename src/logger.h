#ifndef LOGGER_H
#define LOGGER_H

#include "spdlog/spdlog.h"

class logger
{
public:
    enum class type {
        server,
        worker
    };

    using logger_t = std::shared_ptr<spdlog::logger>;

    static logger_t& log(type t) { return console.at(t); };

private:
    static std::map<type, logger_t> console;
};

#endif

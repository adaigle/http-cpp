#ifndef LOGGER_H
#define LOGGER_H

#include "spdlog/spdlog.h"

class logger
{
public:
    using logger_t = std::shared_ptr<spdlog::logger>;

    static logger_t& log() { return console; };

private:
    static logger_t console;
};

#endif

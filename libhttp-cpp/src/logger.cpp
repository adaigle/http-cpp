#include "logger.h"

logger::logger_t logger::console = spdlog::stdout_logger_mt("service", true);

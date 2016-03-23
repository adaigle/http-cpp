#include "http_protocol_handler.hpp"

#include <tuple>

// Include all supported http protocol version
#include "http_protocol_one_one.h"
#include "http_protocol_one_zero.h"

std::unordered_map<std::string, std::unique_ptr<http_protocol_handler>> http_protocol_handler::protocol_handler_cache;

http_protocol_handler* http_protocol_handler::make_handler(const std::string& http_version) noexcept
{
    if (http_version == http_protocol_one_one::http_version)
        return make_handle_impl<http_protocol_one_one>();
    if (http_version == http_protocol_one_zero::http_version)
        return make_handle_impl<http_protocol_one_zero>();

    return nullptr;
}

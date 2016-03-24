#include "http_protocol_handler.hpp"

#include "http_resource_factory.h"

// Include all supported http protocol version
#include "http_protocol_one_one.h"
#include "http_protocol_one_zero.h"

std::unordered_map<std::string, std::unique_ptr<http_protocol_handler>> http_protocol_handler::protocol_handler_cache;

http_protocol_handler::http_protocol_handler(std::unique_ptr<http_resource_factory>&& factory) noexcept :
    resource_factory_(std::move(factory))
{

}

http_protocol_handler* http_protocol_handler::make_handler(const std::string& service_path, const std::string& http_version) noexcept
{
    if (http_version == http_protocol_one_one::http_version)
        return make_handle_impl<http_protocol_one_one>(service_path);
    if (http_version == http_protocol_one_zero::http_version)
        return make_handle_impl<http_protocol_one_zero>(service_path);

    return nullptr;
}

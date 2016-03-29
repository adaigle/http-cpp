#include "http_protocol_handler.hpp"

#include "http_resource_factory.h"

#include  <string>
#include  <sstream>

// Include all supported http protocol version
#include "http_protocol_one_one.h"
#include "http_protocol_one_zero.h"

#include "logger.hpp"

http_protocol_handler::http_protocol_handler() noexcept
{

}

std::string http_protocol_handler::extract_http_version(const std::string& request) noexcept
{
    // Format of HTTP-Version: "HTTP" "/" 1*DIGIT "." 1*DIGIT

    std::string line;
    std::istringstream input(request);
    std::getline(input, line);

    if (line.empty()) return "";
    if (line.back() == '\r') line.pop_back();

    const auto first_space_it = std::find(line.cbegin(), line.cend(), ' ');
    const auto second_space_it = std::find(first_space_it + 1, line.cend(), ' ');
    if (second_space_it != line.cend()) {
        std::string http_version(second_space_it + 1, line.cend());

        return http_version;
    }
    return "";
}

http_protocol_handler* http_protocol_handler::get_handler(http_protocol_handler_cache& cache, const std::string& http_version) noexcept
{
    if (http_version == http_protocol_one_one::http_version)
        return make_handle_impl<http_protocol_one_one>(cache);
    if (http_version == http_protocol_one_zero::http_version)
        return make_handle_impl<http_protocol_one_zero>(cache);

    logger::warn() << "Unknown http version: '" << http_version << "'." << logger::endl;

    return nullptr;
}

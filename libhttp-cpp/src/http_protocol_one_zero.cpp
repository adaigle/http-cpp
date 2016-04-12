#include "http_protocol_one_zero.h"

#include "interface/http_resource.h"
#include "http_resource_factory.h"
#include "http_service.h"

constexpr decltype(http_protocol_one_zero::http_version) http_protocol_one_zero::http_version;


http_protocol_one_zero::http_protocol_one_zero() noexcept :
    http_protocol_handler()
{

}

http_request::parsing_status http_protocol_one_zero::parse_request(const std::string& request, http_request& structured_request) const noexcept
{
    // TODO: Implement the parsing of an HTTP 1.0 request.
    return http_request::parsing_status::success;
}

http_response http_protocol_one_zero::make_response(const generic_response& gresponse) const noexcept
{
    http_response response(gresponse, http_version);

    for (const auto& header_value : gresponse.header) {
        // TODO: Dispatch header based on key.
        response.response_header[header_value.first] = header_value.second;
    }

    response.response_header["Server"] = "http-cpp v0.1";
    response.general_header["Date"] = http_constants::http_date();
    return response;
}

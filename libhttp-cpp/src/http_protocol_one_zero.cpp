#include "http_protocol_one_zero.h"

#include "http_resource.h"
#include "http_resource_factory.h"
#include "http_service.h"

constexpr decltype(http_protocol_one_zero::http_version) http_protocol_one_zero::http_version;


http_protocol_one_zero::http_protocol_one_zero() noexcept :
    http_protocol_handler()
{

}

http_response http_protocol_one_zero::make_response() noexcept
{
    http_response response(http_version);

    response.response_header["Server"] = "http-cpp v0.1";
    response.general_header["Date"] = http_constants::http_date();
    return response;
}

void http_protocol_one_zero::execute(const http_resource_factory* const resource_factory, const http_request& request, http_response& response)
{
    response.status_code = http_constants::status::http_not_implemented;
}

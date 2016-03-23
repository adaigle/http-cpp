#include "http_protocol_one_one.h"

#include <cassert>
#include <exception>
#include <stdexcept>

#include "http_resource.hpp"
#include "http_structure.hpp"

constexpr decltype(http_protocol_one_one::http_version) http_protocol_one_one::http_version;

http_response http_protocol_one_one::make_response() noexcept
{
    http_response response(http_version);

    response.response_header["Server"] = "http-cpp v0.1";
    response.general_header["Date"] = http_constants::http_date();
    return response;
}

void http_protocol_one_one::execute(const http_service_info& info, const http_request& request, http_response& response)
{
    // TODO: Use a static dispatcher to send to the appropriate function ? or use an (abstract) factory ?
    switch (request.method) {
        case http_constants::method::m_options: execute_options(info, request, response); break;
        case http_constants::method::m_get:     execute_get(info, request, response); break;
        case http_constants::method::m_head:    execute_head(info, request, response); break;
        case http_constants::method::m_post:    execute_post(info, request, response); break;
        case http_constants::method::m_put:     execute_put(info, request, response); break;
        case http_constants::method::m_delete:  execute_delete(info, request, response); break;
        case http_constants::method::m_trace:   execute_trace(info, request, response); break;
        default: throw std::invalid_argument("Unsupported method for the current http version.");
    }
}

void http_protocol_one_one::execute_options(const http_service_info& info, const http_request& request, http_response& response)
{
    response.status_code = http_constants::status::http_not_implemented;
}

void http_protocol_one_one::execute_get(const http_service_info& info, const http_request& request, http_response& response)
{
    std::string path = info.path + request.request_uri;
    try {
        http_resource resource = http_resource::get_resource(path);
        http_resource::info meta = resource.fetch_resource_info(info.magic_handle);

        response.entity_header["Content-Type"] = meta.content_type;
        response.entity_header["Content-Length"] = std::to_string(meta.content_length);

        std::ostringstream resource_stream;
        resource.fetch_resource_content(resource_stream);
        response.message_body = resource_stream.str();

        response.status_code = http_constants::status::http_ok;
    } catch (std::exception& e) {
        // Resource cannot be loaded, send out a 404 response.
        response.status_code = http_constants::status::http_not_found;
    }
}

void http_protocol_one_one::execute_head(const http_service_info& info, const http_request& request, http_response& response)
{
    std::string path = info.path + request.request_uri;
    try {
        http_resource resource = http_resource::get_resource(path);
        http_resource::info meta = resource.fetch_resource_info(info.magic_handle);

        response.entity_header["Content-Type"] = meta.content_type;
        response.entity_header["Content-Length"] = std::to_string(meta.content_length);

        response.status_code = http_constants::status::http_ok;
    } catch (std::exception& e) {
        // Resource cannot be loaded, send out a 404 response.
        response.status_code = http_constants::status::http_not_found;
    }
}

void http_protocol_one_one::execute_post(const http_service_info& info, const http_request& request, http_response& response)
{
    response.status_code = http_constants::status::http_not_implemented;
}

void http_protocol_one_one::execute_put(const http_service_info& info, const http_request& request, http_response& response)
{
    response.status_code = http_constants::status::http_not_implemented;
}

void http_protocol_one_one::execute_delete(const http_service_info& info, const http_request& request, http_response& response)
{
    response.status_code = http_constants::status::http_not_implemented;
}

void http_protocol_one_one::execute_trace(const http_service_info& info, const http_request& request, http_response& response)
{
    response.status_code = http_constants::status::http_not_implemented;
}

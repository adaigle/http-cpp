#include "http_protocol_one_one.h"

#include <cassert>
#include <exception>
#include <sstream>
#include <stdexcept>

#include "http_resource.h"
#include "http_resource_factory.h"
#include "http_structure.hpp"

#include "logger.hpp"

constexpr decltype(http_protocol_one_one::http_version) http_protocol_one_one::http_version;

http_protocol_one_one::http_protocol_one_one() noexcept :
    http_protocol_handler()
{
    // Add supported http method.
    dispatcher_.emplace(http_constants::method::m_get, &http_protocol_one_one::execute_get);
    dispatcher_.emplace(http_constants::method::m_head, &http_protocol_one_one::execute_head);
}

http_response http_protocol_one_one::make_response() noexcept
{
    http_response response(http_version);

    response.response_header["Server"] = "http-cpp v0.1";
    response.general_header["Date"] = http_constants::http_date();
    return response;
}

void http_protocol_one_one::execute(const http_resource_factory* const resource_factory, const http_request& request, http_response& response)
{
    const auto it = dispatcher_.find(request.method);
    if (it != dispatcher_.cend()) {
        assert(it->second);
        it->second(this, resource_factory, request, response);
    } else {
        response.status_code = http_constants::status::http_not_implemented;
        response.entity_header["Allow"] = list_implemented_methods();
    }
}

void http_protocol_one_one::execute_get(const http_resource_factory* const resource_factory, const http_request& request, http_response& response) const
{
    try {
        std::unique_ptr<http_resource> resource = resource_factory->create_handle(request.request_uri);
        if (!resource) {
            response.status_code = http_constants::status::http_not_found;
            return;
        }

        const auto header = resource->fetch_resource_header();
        response.entity_header["Content-Type"] = header.at("Content-Type");
        response.entity_header["Content-Length"] = header.at("Content-Length");
        response.entity_header["Last-Modified"] = header.at("Last-Modified");

        std::ostringstream resource_stream;
        resource->fetch_resource_content(resource_stream);
        response.message_body = resource_stream.str();

        response.status_code = http_constants::status::http_ok;
    } catch (std::exception& e) {
        // Resource cannot be loaded, send out a 404 response.
        response.status_code = http_constants::status::http_not_found;
    }
}

void http_protocol_one_one::execute_head(const http_resource_factory* const resource_factory, const http_request& request, http_response& response) const
{
    try {
        std::unique_ptr<http_resource> resource = resource_factory->create_handle(request.request_uri);
        if (!resource) {
            response.status_code = http_constants::status::http_not_found;
            return;
        }

        const auto header = resource->fetch_resource_header();
        response.entity_header["Content-Type"] = header.at("Content-Type");
        response.entity_header["Content-Length"] = header.at("Content-Length");
        response.entity_header["Last-Modified"] = header.at("Last-Modified");

        response.status_code = http_constants::status::http_ok;
    } catch (std::exception& e) {
        // Resource cannot be loaded, send out a 404 response.
        response.status_code = http_constants::status::http_not_found;
    }
}

std::string http_protocol_one_one::list_implemented_methods() const
{
    std::ostringstream ss;
    bool first = true;
    for (const auto& m : dispatcher_) {
        if (!first) ss << ", ";
        else first = false;
        ss << m.first;
    }
    return ss.str();
}

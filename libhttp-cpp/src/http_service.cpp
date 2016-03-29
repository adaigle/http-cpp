///////////////////////////////////////////////////////////
// Forward declarations
#include "http_resource_factory.h"
#include "http_protocol_handler_cache.h"
#include "http_protocol_handler.h"

///////////////////////////////////////////////////////////
// Class declaration
#include "http_service.hpp"

///////////////////////////////////////////////////////////
// Other includes
#include "http_structure.hpp"
#include "http_constants.h"

#include <cassert>
#include <exception>
#include <regex>
#include <sstream>
#include <string>

#include "http_exception.h"

#include "logger.hpp"

std::unique_ptr<http_protocol_handler_cache> http_service::protocol_handler_cache_(std::make_unique<http_protocol_handler_cache>());

http_service::http_service(const std::string& service_path, const std::string& host, const std::string& name /* = "" */) :
    name_(name), host_(host), service_path_(service_path),
    resource_factory_(http_resource_factory::create_resource_factory(service_path))
{
}

// The destructor need to be defined in the .cpp file because at this point, the declaration of 'http_resource_factory'
// is known for the unique_ptr stored in the class.
http_service::~http_service() = default;

http_request http_service::parse_request(const std::string& request)
{
    http_request structured_request;
    const std::string http_version = http_protocol_handler::extract_http_version(request);

    assert(protocol_handler_cache_);
    http_protocol_handler* handler = http_protocol_handler::get_handler(*protocol_handler_cache_.get(), http_version);
    if (handler == nullptr) {
        // TODO: Assume that a wrong/not-implemented http version is a bad request for now.
        throw http_invalid_request("Invalid request.");
    } else {
        http_request::parsing_status code = handler->parse_request(request, structured_request);

        // TODO: Handle parsing return code.
        if (code != http_request::parsing_status::success) {
            throw http_invalid_request("Invalid request.");
        }
    }

    return structured_request;
}

http_response http_service::parse_response(const std::string& response)
{
    // TODO: Implement response parsing to be used by a client.
    throw std::logic_error("Not implemented.");
}

// Simple forwarder to parse_request and execute.
std::string http_service::execute(const std::string& request) const
{
    http_request structured_request = http_service::parse_request(request);

    // TODO: Check that the host match this service's name.

    const http_response structured_response = execute(structured_request);

    std::ostringstream response_stream;
    response_stream << structured_response;
    return response_stream.str();
}

http_response http_service::execute(const http_request& request) const
{
    http_response response;

    try {
        assert(protocol_handler_cache_);
        http_protocol_handler* handler = http_protocol_handler::get_handler(*protocol_handler_cache_.get(), request.http_version);
        if (handler == nullptr) {
            // TODO: Assume that a wrong/not-implemented http version is a bad request for now.
            response.status_code = http_constants::status::http_bad_request;
        } else {
            response = handler->make_response();
            handler->execute(resource_factory_.get(), request, response);
        }
    } catch (std::exception& e) {
        // Resource cannot be loaded, send out a 500 (Internal Server Error) response.
        response.status_code = http_constants::status::http_internal_server_error;
        logger::error() << "Exception while executing the request:" << logger::endl;
        logger::error() << e.what() << logger::endl;
    }

    return response;
}

std::string http_service::extract_host(const http_request& request)
{
    // Check if the Request-URI is an absoluteURI of the following form:
    // "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
    static std::regex absolute_uri_regex("http:\\/\\/([^\\/:]+)(:([0-9]+))?([^\\?]*)(\\?(.*))?", std::regex_constants::ECMAScript | std::regex_constants::optimize);
    static std::regex absolute_path_regex("(\\/([^\\?]*))(\\?(.*))?", std::regex_constants::ECMAScript | std::regex_constants::optimize);

    std::smatch matches;
    if (std::regex_match(request.request_uri, matches, absolute_uri_regex)) {

        // In this case, RFC2616:5.2 says to ignore any host header.
        // We rewrite it in the request_header field.

        const std::string raw_host = matches[1];
        const std::string raw_port = matches[3];
        const std::string raw_abs_path = matches[4];
        const std::string raw_query = matches[6];

        logger::trace() << "Absolute URI detected: " << logger::endl;
        logger::trace() << " - Host: " << raw_host << "(" << raw_port << ")" << logger::endl;
        logger::trace() << " - Abs_path: " << raw_abs_path << logger::endl;
        logger::trace() << " - Query: " << raw_query << logger::endl;

        return raw_host;
    }

    if (std::regex_match(request.request_uri, matches, absolute_path_regex)) {
        // Make sure that the host header exists.
        const auto host_it = request.request_header.find("Host");
        if (host_it == request.request_header.cend()) {
            logger::warn() << "The 'Host' header must be provided for absolute-path requests." << logger::endl;
            throw http_invalid_request("'Host' header is missing.");
        }

        const std::string raw_abs_path = matches[1];
        const std::string raw_query = matches[4];

        logger::trace() << "Absolute path detected: " << logger::endl;
        logger::trace() << " - Host: " << host_it->second << logger::endl;
        logger::trace() << " - Abs_path: " << raw_abs_path << logger::endl;
        logger::trace() << " - Query: " << raw_query << logger::endl;

        return host_it->second;
    }

    if (request.request_uri == "*") {
        // Nothing to do.
        return "*";
    }

    // TODO: Handle 'authority'-based request.

    logger::warn() << "Invalid Request-URI: " << request.request_uri << logger::endl;
    throw std::invalid_argument("Invalid Request-URI parameter in the request, could not detect a valid format.");
}

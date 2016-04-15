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

#include "logger.h"

std::unique_ptr<http_protocol_handler_cache> http_service::protocol_handler_cache_(std::make_unique<http_protocol_handler_cache>());

http_service::http_service(const std::string& service_path, host&& host, const std::string& name /* = "" */) :
    name_(name), host_(std::move(host)), service_path_(service_path),
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
    // Execute an http request as follows:
    // 1. Identify http version & get handle to parser.
    // 2. Parse http request.
    // 3. Identify web service.
    // 4. Create generic request & response.
    // 5. Forward request to specific service.
    // 6. Create http response based on generic response.

    // Note: Only http_protocol_* classes are aware of http request/response format.
    // Note: Concrete services (REST, filesystem, etc..) only deal with generic request/response.

    // At this point, the http request if properly formed already.

    ///////////////////////////////////////////////////
    // 3. Identify web service.
    const host detected_host = extract_host(request);
    if (detected_host != host_)
        logger::log()->warn() << "Non-matching host, expected '" << host_ << "' but got '" << detected_host << "'.";

    ///////////////////////////////////////////////////
    // 4. Create generic request & response.
    generic_request grequest = request.to_generic();
    generic_response gresponse;

    assert(protocol_handler_cache_);
    http_protocol_handler* handler = http_protocol_handler::get_handler(*protocol_handler_cache_.get(), request.http_version);
    if (handler == nullptr) {
        gresponse.status_code = http_constants::status::http_bad_request;
    } else {
        try {
            ///////////////////////////////////////////////
            // 5. Forward request to specific service.
            assert(resource_factory_);
            std::unique_ptr<http_resource> resource = resource_factory_->create_handle(grequest);
            if (!resource) {
                gresponse.status_code = http_constants::status::http_not_found;
                logger::log()->warn() << "Resource not found.";
            } else {
                resource->execute(grequest, gresponse);
            }

        } catch (http_invalid_request& e) {
            gresponse.status_code = http_constants::status::http_bad_request;
            logger::log()->warn() << "Bad request: " << e.what();
        } catch (std::exception& e) {
            // Resource cannot be loaded, send out a 500 (Internal Server Error) response.
            gresponse.status_code = http_constants::status::http_internal_server_error;
            logger::log()->error() << "Exception while executing the request:";
            logger::log()->error() << e.what();
        }
    }

    // Assume successful result by default.
    if (gresponse.status_code == http_constants::status::http_unknown) {
        gresponse.status_code = http_constants::status::http_ok;
    }

    ///////////////////////////////////////////////////
    // 6. Create http response based on generic response.
    http_response response;
    if (handler == nullptr) {
        response.status_code = gresponse.status_code;
    } else {
        response = handler->make_response(gresponse);
    }

    return response;
}

http_service::host http_service::extract_host(const http_request& request)
{
    // Check if the Request-URI is an absoluteURI of the following form:
    // "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
    static std::regex absolute_uri_regex("http:\\/\\/([^\\/:]+)(:([0-9]+))?([^\\?]*)(\\?(.*))?", std::regex_constants::ECMAScript | std::regex_constants::optimize);
    static std::regex absolute_path_regex("(\\/([^\\?]*))(\\?(.*))?", std::regex_constants::ECMAScript | std::regex_constants::optimize);
    static std::regex host_regex("([^\\/:]+)(:([0-9]+))?", std::regex_constants::ECMAScript | std::regex_constants::optimize);

    std::smatch matches;
    if (std::regex_match(request.request_uri, matches, absolute_uri_regex)) {

        // In this case, RFC2616:5.2 says to ignore any host header.
        // We rewrite it in the request_header field.

        const std::string raw_host = matches[1];
        const std::string raw_port = matches[3];
        const std::string raw_abs_path = matches[4];
        const std::string raw_query = matches[6];

        logger::log()->trace() << "Absolute URI detected: ";
        logger::log()->trace() << " - Host: " << raw_host << "(" << raw_port << ")";
        logger::log()->trace() << " - Abs_path: " << raw_abs_path;
        logger::log()->trace() << " - Query: " << raw_query;

        // Set default port to 80.
        uint16_t port;
        std::istringstream ss(raw_port);
        ss >> port;
        if (!ss) port = 80;

        return host{raw_host, port};
    }

    if (std::regex_match(request.request_uri, matches, absolute_path_regex)) {
        // Make sure that the host header exists.
        const auto host_it = request.request_header.find("Host");
        if (host_it == request.request_header.cend()) {
            logger::log()->warn() << "The 'Host' header must be provided for absolute-path requests.";
            throw http_invalid_request("'Host' header is missing.");
        }

        const std::string raw_abs_path = matches[1];
        const std::string raw_query = matches[4];

        logger::log()->trace() << "Absolute path detected: ";
        logger::log()->trace() << " - Host: " << host_it->second;
        logger::log()->trace() << " - Abs_path: " << raw_abs_path;
        logger::log()->trace() << " - Query: " << raw_query;

        std::smatch host_matches;
        if (std::regex_match(host_it->second, host_matches, host_regex)) {
            const std::string raw_host = host_matches[1];
            const std::string raw_port = host_matches[3];

            // Set default port to 80.
            uint16_t port;
            std::istringstream ss(raw_port);
            ss >> port;
            if (!ss) port = 80;

            return host{raw_host, port};
        }
    }

    if (request.request_uri == "*") {
        // Nothing to do.
        return host{"*", 80};
    }

    // TODO: Handle 'authority'-based request.

    logger::log()->warn() << "Invalid Request-URI: " << request.request_uri;
    throw std::invalid_argument("Invalid Request-URI parameter in the request, could not detect a valid format.");
}

///////////////////////////////////////////////////////////
// class http_service::host
///////////////////////////////////////////////////////////

http_service::host::host(const std::string& n, const uint16_t p) noexcept :
    name(n), port(p)
{
}

bool http_service::host::match(const std::string& host) const
{
    bool is_match = false;

    is_match |= (host == name);

    const std::string fullhost = name + ":" + std::to_string(port);
    is_match |= (host == fullhost);

    return is_match;
}

bool http_service::host::operator==(const host& other) const
{
    return port == other.port && name == other.name;
}

bool http_service::host::operator!=(const host& other) const
{
    return port != other.port || name != other.name;
}

bool http_service::host::operator<(const host& other) const
{
    return port < other.port || (port == other.port && name < other.name);
}

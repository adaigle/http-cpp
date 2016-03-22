#include "http_service.h"

#include <exception>
#include <regex>
#include <sstream>
#include <string>

#include <boost/filesystem.hpp>

#include "http_exception.h"
#include "http_resource.hpp"

#include "logger.hpp"

#if defined(HAVE_LIBMAGIC)
    magic_t http_service::magic_handle_ = nullptr;
#endif

http_service::http_service(const std::string& name, const std::string& path) :
    name_(name), path_(path), method_dispatch_(dispatch_construction())
{
#if defined(HAVE_LIBMAGIC)
    if (magic_handle_ == nullptr) {
        magic_handle_ = ::magic_open(MAGIC_ERROR | MAGIC_MIME);

        if (!boost::filesystem::exists(LIBMAGIC_MAGIC_FILE)) {
            logger::error() << "Database for libmagic could not be found at '" << LIBMAGIC_MAGIC_FILE << "'." << logger::endl;
            magic_handle_ = nullptr;
        } else {
            logger::trace() << "Loading magic database..." << logger::endl;
            ::magic_load(magic_handle_, LIBMAGIC_MAGIC_FILE);

            const char* error = ::magic_error(magic_handle_);
            if (error != nullptr) logger::warn() << "libmagic error: " << error << logger::endl;
            else logger::debug() << "Successfully loaded magic database." << logger::endl;
        }
    }
#endif
}

http_service::~http_service()
{
    // TODO: Handle a better single owner of magic_handle_.
    // ::magic_close(magic_handle_);
}

http_request http_service::parse_request(const std::string& request)
{
    http_request structured_request;
    http_request::parsing_status code = structured_request.parse(request);

    // TODO: Handle parsing return code.
    if (code != http_request::parsing_status::success) {
        throw http_invalid_request("Invalid request.");
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
    response.http_version = request.http_version;
    response.message_body = "";
    const std::string host = extract_host(request);

    // TODO: Check that the host match the request.
    // ...

    try {
        const auto it_func = method_dispatch_.find(execution_handler_identifier{request.http_version, request.method});
        if (it_func == method_dispatch_.cend()) {

            response.status_code = 501;
        } else {
            (it_func->second)(this, request, response);
        }
    } catch (std::exception& e) {
        // Resource cannot be loaded, send out a 500 (Internal Server Error) response.
        response.status_code = 500;
    }

    return response;
}

void http_service::execute_options(const http_request& request, http_response& response) const
{
    response.status_code = 501;
}

void http_service::execute_get(const http_request& request, http_response& response) const
{
    std::string path = path_ + request.request_uri;
    try {
        http_resource resource = http_resource::get_resource(path);
        http_resource::info meta = resource.fetch_resource_info(magic_handle_);

        response.entity_header["Content-Type"] = meta.content_type;
        response.entity_header["Content-Length"] = std::to_string(meta.content_length);

        std::ostringstream resource_stream;
        resource.fetch_resource_content(resource_stream);
        response.message_body = resource_stream.str();

        response.status_code = 200;
    } catch (std::exception& e) {
        // Resource cannot be loaded, send out a 404 response.
        response.status_code = 404;
    }
}

void http_service::execute_head(const http_request& request, http_response& response) const
{
    std::string path = path_ + request.request_uri;
    try {
        http_resource resource = http_resource::get_resource(path);
        http_resource::info meta = resource.fetch_resource_info(magic_handle_);

        response.entity_header["Content-Type"] = meta.content_type;
        response.entity_header["Content-Length"] = std::to_string(meta.content_length);

        response.status_code = 200;
    } catch (std::exception& e) {
        // Resource cannot be loaded, send out a 404 response.
        response.status_code = 404;
    }
}

void http_service::execute_post(const http_request& request, http_response& response) const
{
    response.status_code = 501;
}

void http_service::execute_put(const http_request& request, http_response& response) const
{
    response.status_code = 501;
}

void http_service::execute_delete(const http_request& request, http_response& response) const
{
    response.status_code = 501;
}

void http_service::execute_trace(const http_request& request, http_response& response) const
{
    response.status_code = 501;
}

std::string http_service::extract_host(const http_request& request)
{
    const auto host_it = request.request_header.find("Host");
    std::string host = (host_it != request.request_header.cend()) ? host_it->second : "";

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
        const std::string raw_abs_path = matches[1];
        const std::string raw_query = matches[4];

        logger::trace() << "Absolute path detected: " << logger::endl;
        logger::trace() << " - Abs_path: " << raw_abs_path << logger::endl;
        logger::trace() << " - Query: " << raw_query << logger::endl;

        return host;
    }

    if (request.request_uri == "*") {
        // Nothing to do.
        return "*";
    }

    // TODO: Handle 'authority'-based request.

    logger::warn() << "Invalid Request-URI: " << request.request_uri << logger::endl;
    throw std::invalid_argument("Invalid Request-URI parameter in the request, could not detect a valid format.");
}

http_service::exec_dispatch_t http_service::dispatch_construction()
{
    exec_dispatch_t dispatcher;
    dispatcher.emplace(execution_handler_identifier{"HTTP/1.1", http_constants::method::m_options}, &http_service::execute_options);
    dispatcher.emplace(execution_handler_identifier{"HTTP/1.1", http_constants::method::m_get}, &http_service::execute_get);
    //dispatcher.emplace(execution_handler_identifier{"HTTP/1.1", http_constants::method::m_head}, &http_service::execute_head);
    dispatcher.emplace(execution_handler_identifier{"HTTP/1.1", http_constants::method::m_post}, &http_service::execute_post);
    dispatcher.emplace(execution_handler_identifier{"HTTP/1.1", http_constants::method::m_put}, &http_service::execute_put);
    dispatcher.emplace(execution_handler_identifier{"HTTP/1.1", http_constants::method::m_delete}, &http_service::execute_delete);
    dispatcher.emplace(execution_handler_identifier{"HTTP/1.1", http_constants::method::m_trace}, &http_service::execute_trace);

    return dispatcher;
}

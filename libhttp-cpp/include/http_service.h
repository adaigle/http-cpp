#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

#include <memory>
#include <string>

#include "http_structure.h"

// Forward declaration of the http resource factory.
class http_resource_factory;

// Forward declaration of the http protocol handler cache.
class http_protocol_handler_cache;

/// \brief Provide parsing and execution capacities of http request/response.
///
class http_service
{
public:
    /// \brief Constructor of an http service.
    ///
    /// \param service_path Path to the website / web service.
    /// \param name Internal name of the service.
    /// \param host Host of the http service (external name).
    http_service(const std::string& service_path, const std::string& host, const std::string& name = "");

    /// \brief Default destructor.
    ~http_service();

    /// \brief Parse an http request from a string.
    ///
    /// \param request The http request.
    /// \returns The structured http request.
    static http_request parse_request(const std::string& request);

    /// \brief Parse an http response from a string.
    ///
    /// \param request The http response.
    /// \returns The structured http response.
    static http_response parse_response(const std::string& response);

    /// \brief Execute an http request as string and returns a response as string.
    ///
    /// \param request The http request.
    /// \returns The http response given as string.
    std::string execute(const std::string& request) const;

    /// \brief Execute a structured http request and returns a structured response.
    ///
    /// \param request The http request.
    /// \returns The http response given as an object.
    http_response execute(const http_request& request) const;

    /// \brief Return the host name of an http request.
    ///
    /// \param request The http request.
    /// \returns The hostname found in the request.
    static std::string extract_host(const http_request&);

protected:
    http_service() = delete;
    http_service(const http_service&) = delete;
    http_service& operator=(const http_service&) = delete;

    const std::string name_;
    const std::string host_;
    const std::string service_path_;

    std::unique_ptr<http_resource_factory> resource_factory_;
    static std::unique_ptr<http_protocol_handler_cache> protocol_handler_cache_;
};

#endif

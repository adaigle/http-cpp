#ifndef HTTP_PROTOCOL_HANDLER_H
#define HTTP_PROTOCOL_HANDLER_H

#include <memory>
#include <string>
#include <unordered_map>

#include "http_structure.hpp"

class http_protocol_handler
{
public:
    /// \brief Obtain a handle to a version-specific executor of the http protocol.
    /// make_handler lazily creates a single static instance of the concrete strategy
    /// used to answer to requests of a specific profocol version. When the concrete
    /// instance is created once, it is stored and reused for subsequent requests.
    /// \note http_protocol_handler handles dynamic allocation.
    ///
    /// \param http_version The string matching the http version. e.g.: 'HTTP/1.1'.
    /// \returns A pointer to the protocol handler or nullptr if the version is invalid.
    static http_protocol_handler* make_handler(const std::string& http_version) noexcept;

    /// \brief Creates a basic response for a specific protocol version.
    ///
    /// \returns A valid default response for the protocol version.
    virtual http_response make_response() noexcept = 0;

    /// \brief Execute the request for the specific protocol version and returns
    /// \note Any exception thrown by the implementation leads to a internal server error.
    ///
    /// \param env Information about the service settings.
    /// \param request An http request ni the http version of the concete instance.
    /// \param response The response to populate.
    virtual void execute(const http_service_info& info, const http_request& request, http_response& response) = 0;

private:
    template <typename T>
    static http_protocol_handler* make_handle_impl() noexcept;

    static std::unordered_map<std::string, std::unique_ptr<http_protocol_handler>> protocol_handler_cache;
};

#endif

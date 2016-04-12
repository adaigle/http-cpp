#ifndef HTTP_PROTOCOL_HANDLER_H
#define HTTP_PROTOCOL_HANDLER_H

#include <memory>
#include <string>
#include <unordered_map>

#include "http_protocol_handler_cache.h"
#include "http_structure.h"

//Forward declaration of the resource factory.
class http_resource_factory;

class http_protocol_handler
{
protected:
    http_protocol_handler() noexcept;
public:
    virtual ~http_protocol_handler() = default;


    /// \brief Get the protocol version from the entire http request.
    ///
    /// \param request The entire http request in string.
    /// \returns The http version extracted from the requets or empty if the version could not be found.
    static std::string extract_http_version(const std::string& request) noexcept;

    /// \brief Obtain a handle to a version-specific executor of the http protocol.
    /// make_handler lazily creates a single static instance of the concrete strategy
    /// used to answer to requests of a specific profocol version. When the concrete
    /// instance is created once, it is stored and reused for subsequent requests.
    /// \note http_protocol_handler handles dynamic allocation.
    ///
    /// \param http_version The string matching the http version. e.g.: 'HTTP/1.1'.
    /// \returns A pointer to the protocol handler or nullptr if the version is invalid.
    static http_protocol_handler* get_handler(http_protocol_handler_cache& cache, const std::string& http_version) noexcept;

    /// \brief Parse an http request according to the version.
    ///
    /// \param request The entire http request in string.
    /// \param structured_request The structured request to fill during parsing.
    /// \returns The parsing status.
    virtual http_request::parsing_status parse_request(const std::string& request, http_request& structured_request) const noexcept = 0;

    /// \brief Creates a basic response for a specific protocol version.
    ///
    /// \returns A valid default response for the protocol version.
    virtual http_response make_response(const generic_response& gresponse) const noexcept = 0;

private:
    template <typename T>
    static http_protocol_handler* make_handle_impl(http_protocol_handler_cache& cache) noexcept;
};

#endif

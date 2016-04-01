#ifndef HTTP_PROTOCOL_ONE_ONE_H
#define HTTP_PROTOCOL_ONE_ONE_H

#include "http_protocol_handler.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

class http_protocol_one_one : public http_protocol_handler
{
public:
    static constexpr auto http_version = "HTTP/1.1";

    http_protocol_one_one() noexcept;
    virtual ~http_protocol_one_one() = default;

    /// \brief Parse an http request according to the version.
    ///
    /// \param request The entire http request in string.
    /// \param structured_request The structured request to fill during parsing.
    /// \returns The parsing status.
    virtual http_request::parsing_status parse_request(const std::string& request, http_request& structured_request) noexcept override;

    /// \brief Creates a basic response for a specific protocol version.
    ///
    /// \returns A valid default response for the protocol version.
    virtual http_response make_response() noexcept override;

    /// \brief Execute the request for the specific protocol version and returns
    /// \note Any exception thrown by the implementation leads to a internal server error.
    ///
    /// \param resource_factory Information about the service settings.
    /// \param request An http request ni the http version of the concete instance.
    /// \param response The response to populate.
    virtual void execute(const http_resource_factory* const resource_factory, const http_request& request, http_response& response) override;

protected:
    virtual void execute_get(const http_resource_factory* const resource_factory, const http_request& request, http_response& response) const;
    virtual void execute_head(const http_resource_factory* const resource_factory, const http_request& request, http_response& response) const;

private:
    std::string list_implemented_methods() const;

    using execute_fn_signature = void(const http_protocol_one_one* const, const http_resource_factory* const, const http_request&, http_response&);

    std::map<http_constants::method,
             std::function<execute_fn_signature>> dispatcher_;
};

#endif

#ifndef HTTP_PROTOCOL_ONE_ZERO_H
#define HTTP_PROTOCOL_ONE_ZERO_H

#include "http_protocol_handler.h"

#include <memory>

class http_protocol_one_zero : public http_protocol_handler
{
public:

    static constexpr auto http_version = "HTTP/1.0";

    http_protocol_one_zero() noexcept;
    virtual ~http_protocol_one_zero() = default;

    /// \brief Creates a basic response for a specific protocol version.
    ///
    /// \returns A valid default response for the protocol version.
    virtual http_response make_response() noexcept override;

    /// \brief Execute the request for the specific protocol version and returns
    /// \note Any exception thrown by the implementation leads to a internal server error.
    ///
    /// \param env Information about the service settings.
    /// \param request An http request ni the http version of the concete instance.
    /// \param response The response to populate.
    virtual void execute(const http_resource_factory* const resource_factory, const http_request& request, http_response& response) override;

};

#endif

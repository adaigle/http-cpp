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

    /// \brief Parse an http request according to the version.
    ///
    /// \param request The entire http request in string.
    /// \param structured_request The structured request to fill during parsing.
    /// \returns The parsing status.
    virtual http_request::parsing_status parse_request(const std::string& request, http_request& structured_request) const noexcept override;

    /// \brief Creates a basic response for a specific protocol version.
    ///
    /// \returns A valid default response for the protocol version.
    virtual http_response make_response(const generic_response& gresponse) const noexcept override;

};

#endif

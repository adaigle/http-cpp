#ifndef HTTP_PROTOCOL_ONE_ONE_H
#define HTTP_PROTOCOL_ONE_ONE_H

#include "http_protocol_handler.h"

class http_protocol_one_one : public http_protocol_handler
{
public:
    static constexpr auto http_version = "HTTP/1.1";

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
    virtual void execute(const http_service_info& info, const http_request& request, http_response& response) override;

protected:
    virtual void execute_options(const http_service_info& info, const http_request& request, http_response& response);
    virtual void execute_get(const http_service_info& info, const http_request& request, http_response& response);
    virtual void execute_head(const http_service_info& info, const http_request& request, http_response& response);
    virtual void execute_post(const http_service_info& info, const http_request& request, http_response& response);
    virtual void execute_put(const http_service_info& info, const http_request& request, http_response& response);
    virtual void execute_delete(const http_service_info& info, const http_request& request, http_response& response);
    virtual void execute_trace(const http_service_info& info, const http_request& request, http_response& response);
};

#endif

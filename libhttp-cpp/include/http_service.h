#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

#include <memory>
#include <string>

#include "http_service.hpp"
#include "http_structure.hpp"

#if defined(HAVE_LIBMAGIC)
#  if !defined(LIBMAGIC_MAGIC_FILE)
#    error "The path to libmagic folder must be provided."
#  endif
#  include "magic.h"
#endif

/// \brief Provide parsing and execution capacities of http request/response.
///
class http_service
{
public:
    http_service(const std::string& name, const std::string& path);
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

protected:
    static std::string extract_host(const http_request&);

private:
    const std::string name_;
    std::string path_;

    http_service_info environment;

#if defined(HAVE_LIBMAGIC)
    using magic_up = std::unique_ptr<magic_set, magic_deleter>;
    static magic_up up_magic_handle_;
#endif
};

#endif

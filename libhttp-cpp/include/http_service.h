#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

#include <functional>
#include <string>
#include <unordered_map>
#include <type_traits>

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
    void execute_options(const http_request&, http_response&) const;
    void execute_get(const http_request&, http_response&) const;
    void execute_head(const http_request&, http_response&) const;
    void execute_post(const http_request&, http_response&) const;
    void execute_put(const http_request&, http_response&) const;
    void execute_delete(const http_request&, http_response&) const;
    void execute_trace(const http_request&, http_response&) const;

    static std::string extract_host(const http_request&);

private:

    using exec_dispatch_signature_t = void(const http_service* const, const http_request&, http_response&);
    using exec_dispatch_t = std::unordered_map<execution_handler_identifier,
                                               std::function<exec_dispatch_signature_t>>;

    exec_dispatch_t dispatch_construction();

    const std::string name_;
    std::string path_;
    const exec_dispatch_t method_dispatch_;

#if defined(HAVE_LIBMAGIC)
    static magic_t magic_handle_;
#endif
};

#endif

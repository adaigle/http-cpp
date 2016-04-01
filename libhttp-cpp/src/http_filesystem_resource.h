#ifndef HTTP_FILESYSTEM_RESOURCE_H
#define HTTP_FILESYSTEM_RESOURCE_H

#include "interface/http_resource.h"
#include "http_structure.h"

#include <fstream>
#include <map>
#include <string>

#if defined(HAVE_LIBMAGIC)
#  include "magic.h"
    using magic_handle_t = magic_set*;
#else
    using magic_handle_t = void*;
#endif

class http_filesystem_resource : public http_resource
{
public:
    using header_t = std::map<std::string, std::string>;

    http_filesystem_resource(const std::string& request_uri, magic_handle_t magic_handle);

    /// \brief Execute the request on the resource.
    ///
    /// \param request The request to execute.
    /// \param response The response to fill in.
    virtual void execute(const http_request& request, http_response& response) override final;

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual header_t fetch_resource_header();

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual void fetch_resource_content(std::ostream& stream);

protected:
    std::string get_content_type();

    std::ifstream stream_;
    magic_handle_t magic_handle_;
};

#endif

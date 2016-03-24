#ifndef HTTP_FILESYSTEM_RESOURCE_H
#define HTTP_FILESYSTEM_RESOURCE_H

#include "http_resource.h"

#include <fstream>

#if defined(HAVE_LIBMAGIC)
#  include "magic.h"
    using magic_handle_t = magic_set*;
#else
    using magic_handle_t = void*;
#endif

class http_filesystem_resource : public http_resource
{
public:
    http_filesystem_resource(const std::string& request_uri, magic_handle_t magic_handle);

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual header_t fetch_resource_header() override;

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual void fetch_resource_content(std::ostream& stream) override;

protected:
    std::string get_content_type();

    std::ifstream stream_;
    magic_handle_t magic_handle_;
};

#endif

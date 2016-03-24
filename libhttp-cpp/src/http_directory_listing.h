#ifndef HTTP_DIRECTORY_LISTING_H
#define HTTP_DIRECTORY_LISTING_H

#include "http_filesystem_resource.h"

class http_directory_listing : public http_filesystem_resource
{
public:
    http_directory_listing(const std::string& request_uri, magic_handle_t magic_handle);

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual header_t fetch_resource_header() override;

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual void fetch_resource_content(std::ostream& stream) override;
};

#endif

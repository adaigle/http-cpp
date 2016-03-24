#ifndef HTTP_EXTERNAL_RESOURCE_H
#define HTTP_EXTERNAL_RESOURCE_H

#include "http_resource.h"

#include <string>

class http_external_resource : public http_resource
{
public:
    http_external_resource(const std::string& request_uri);

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

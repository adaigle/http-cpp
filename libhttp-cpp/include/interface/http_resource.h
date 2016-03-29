#ifndef HTTP_RESOURCE_H
#define HTTP_RESOURCE_H

#include <map>
#include <ostream>
#include <string>

class http_resource
{
public:
    using header_t = std::map<std::string, std::string>;

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual header_t fetch_resource_header() = 0;

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual void fetch_resource_content(std::ostream& stream) = 0;

protected:
    http_resource(const std::string& request_uri) : request_uri_(request_uri) {}

    const std::string request_uri_;
};

#endif

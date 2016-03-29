#ifndef REST_RESOURCE_H
#define REST_RESOURCE_H

#include "interface/http_resource.h"

#include <string>

#include <boost/dll/alias.hpp>

class rest_resource : public http_resource
{
public:
    rest_resource(const std::string& request_uri);

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual header_t fetch_resource_header() override;

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual void fetch_resource_content(std::ostream& stream) override;

    static http_resource* create_handle(const std::string str) {
        return new rest_resource(str);
    }
};

BOOST_DLL_ALIAS(rest_resource::create_handle, create_handle);

#endif

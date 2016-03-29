#ifndef CUSTOM_RESOURCE_H
#define CUSTOM_RESOURCE_H

#include "interface/http_resource.h"

#include <string>

#include <boost/dll/alias.hpp>

class custom_resource : public http_resource
{
public:
    custom_resource(const std::string& request_uri);

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual header_t fetch_resource_header() override;

    /// \brief Fetch the resource content in a stream format..
    ///
    /// \param stream The stream to output the content to.
    virtual void fetch_resource_content(std::ostream& stream) override;

    static http_resource* create_handle(const std::string str) {
        return new custom_resource(str);
    }
};

BOOST_DLL_ALIAS(custom_resource::create_handle, create_handle);

#endif

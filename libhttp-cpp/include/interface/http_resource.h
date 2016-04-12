#ifndef HTTP_RESOURCE_H
#define HTTP_RESOURCE_H

#include <map>
#include <ostream>
#include <string>

#include "generic_structure.h"

/// \brief Base definition of an http resource.
///
class http_resource
{
public:
    /// \brief Execute the request on the resource.
    ///
    /// \param request The request to execute.
    /// \param response The response to fill in.
    virtual void execute(const generic_request& request, generic_response& response) = 0;

protected:
    http_resource(const std::string& request_uri) : request_uri_(request_uri) {}

    const std::string request_uri_;
};

#endif

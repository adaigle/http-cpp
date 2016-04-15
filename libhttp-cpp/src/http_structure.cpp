///////////////////////////////////////////////////////////
// Class declaration
#include "http_structure.hpp"

///////////////////////////////////////////////////////////
// Other includes
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include <tuple>

generic_request http_request::to_generic() const
{
    generic_request grequest(method, request_uri, message_body);
    grequest.header.insert(general_header.cbegin(), general_header.cend());
    grequest.header.insert(request_header.cbegin(), request_header.cend());
    grequest.header.insert(entity_header.cbegin(), entity_header.cend());

    return grequest;
}

http_response::http_response(const generic_response& gresponse, const std::string http_version) noexcept :
    http_version(http_version), status_code(gresponse.status_code), message_body(gresponse.message_body)
{
}

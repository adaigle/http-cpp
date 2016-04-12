#ifndef GENERIC_STRUCTURE_H
#define GENERIC_STRUCTURE_H

#include <map>
#include <string>

#include "http_constants.h"

struct generic_request {
    using header_map = std::map<std::string, std::string>;

    generic_request(http_constants::method m, const std::string& request_uri, const std::string& message_body) :
        method(m), request_uri(request_uri), message_body(message_body) {};

    http_constants::method method;
    const std::string&     request_uri;
    header_map             header;
    const std::string&     message_body;
};

struct generic_response {
    using header_map = std::map<std::string, std::string>;

    generic_response() : message_body_complete(true) {};

    http_constants::status status_code;
    header_map             header;
    std::string            message_body;
    bool                   message_body_complete;
};

#endif

#ifndef HTTP_STRUCTURES_H
#define HTTP_STRUCTURES_H

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <map>
#include <sstream>
#include <string>

#include "http_constants.h"

#if defined(HAVE_LIBMAGIC)
#  include "magic.h"
#endif

struct http_request
{
    enum class parsing_status : uint8_t {
        success = 0,
        empty_request,
        invalid_request_line,
        invalid_method
    };

    using header_map = std::map<std::string, std::string>;

    ///////////////////////////////////////////////////////
    // HTTP Request format:
    //   Request      = Request-Line CRLF
    //                  *(( general-header | request-header | entity-header ) CRLF) CRLF
    //                  [ message-body ]
    //
    //   Request-Line = Method SP Request-URI SP HTTP-Version
    //
    //   Method       = "OPTIONS" | "GET" | "HEAD" | "POST" | "PUT" | "DELETE" | "TRACE" | "CONNECT" | extension-method
    //
    //   Request-URI  = "*" | absoluteURI | abs_path | authority
    //
    // Example:
    //   GET /pub/WWW/TheProject.html HTTP/1.1
    //   Host: www.w3.org
    ///////////////////////////////////////////////////////

    http_constants::method method;
    std::string request_uri;
    std::string http_version;
    header_map  general_header;
    header_map  request_header;
    header_map  entity_header;
    std::string message_body;
};

struct http_response
{
    using header_map = std::map<std::string, std::string>;

    ///////////////////////////////////////////////////////
    // HTTP Request format:
    //   Response     = Status-Line CRLF
    //                  *(( general-header | response-header | entity-header ) CRLF) CRLF
    //                  [ message-body ]
    //
    //   Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase
    //
    // Example:
    //   HTTP/1.0 200 OK
    //   Content-Type: text/plain
    //
    //   Hello, World!
    ///////////////////////////////////////////////////////

    std::string http_version;
    http_constants::status status_code;
    header_map  general_header;
    header_map  response_header;
    header_map  entity_header;
    std::string message_body;

    http_response(const std::string http_version = "HTTP/1.1") : http_version(http_version) {}
};

#endif

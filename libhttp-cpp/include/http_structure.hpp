#ifndef HTTP_STRUCTURES_HPP
#define HTTP_STRUCTURES_HPP

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <map>
#include <sstream>
#include <string>

#if defined(HAVE_LIBMAGIC)
#  include "magic.h"
#endif

struct http_constants
{
    enum class method : uint8_t {
        m_options = 0,
        m_get     = 1,
        m_head    = 2,
        m_post    = 3,
        m_put     = 4,
        m_delete  = 5,
        m_trace   = 6,
        m_connect = 7
    };

    enum class status_class : uint8_t {
        informational = 1,
        success = 2,
        redirection = 3,
        client_error = 4,
        server_error = 5
    };

    enum class status : uint16_t {
        http_continue = 100,
        http_switching_protocols = 101,

        http_ok = 200,
        http_created = 201,
        http_accepted = 202,
        http_nonauthoritative = 203,
        http_no_content = 204,
        http_reset_content = 205,
        http_partial_content = 206,

        http_multiple_choices = 300,
        http_moved_permanently = 301,
        http_found = 302,
        http_see_other = 303,
        http_not_modified = 304,
        http_use_proxy = 305,
        http_temporary_redirect = 307,

        http_bad_request = 400,
        http_unhautorized = 401,
        http_payment_required = 402,
        http_forbidden = 403,
        http_not_found = 404,
        http_method_not_allowed = 405,
        http_not_acceptable = 406,
        http_proxy_authentication_required = 407,
        http_request_timeout = 408,
        http_conflict = 409,
        http_gone = 410,
        http_length_required = 411,
        http_precondition_failed = 412,
        http_request_entry_too_large = 413,
        http_requesturi_too_large = 414,
        http_unsupported_media_type = 415,
        http_requested_range_not_satisfiable = 416,
        http_expectation_failed = 417,

        http_internal_server_error = 500,
        http_not_implemented = 501,
        http_bad_gateway = 502,
        http_service_unavailable = 503,
        http_gateway_timeout = 504,
        http_version_not_supported = 505
    };

    static constexpr char SP = ' ';
    static constexpr char CM = ',';
    static constexpr auto CRLF = "\r\n";

    static constexpr auto METHODS = {"OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT"};
    static constexpr auto GENERAL_HEADER = {"Cache-Control", "Connection", "Date", "Pragma", "Trailer",
                                            "Transfer-Encoding", "Upgrade", "Via", "Warning"};
    static constexpr auto REQUEST_HEADER = {"Accept", "Accept-Charset", "Accept-Encoding", "Accept-Language",
                                            "Authorization", "Expect", "From", "Host", "If-Match", "If-Modified-Since",
                                            "If-None-Match", "If-Range", "If-Unmodified-Since", "Max-Forwards",
                                            "Proxy-Authorization", "Range", "Referer", "TE", "User-Agent"};
    static constexpr auto ENTITY_HEADER = {"Allow", "Content-Encoding", "Content-Language", "Content-Length",
                                           "Content-Location", "Content-MD5", "Content-Range", "Content-Type",
                                           "Expires", "Last-Modified"};

    static std::string reason_phrase(status code);
    static status_class get_status_class(status code);

    static std::string http_date();
};

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

    parsing_status parse(const std::string& frame);

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

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<< (std::basic_ostream<CharT, Traits>& stream, const http_response& response)
{
    stream << response.http_version << http_constants::SP
           << static_cast<std::underlying_type_t<decltype(response.status_code)>>(response.status_code) << http_constants::SP
           << http_constants::reason_phrase(response.status_code) << http_constants::CRLF;

    for (const auto& h : response.general_header)
        stream << h.first << ": " << h.second << http_constants::CRLF;
    for (const auto& h : response.response_header)
        stream << h.first << ": " << h.second << http_constants::CRLF;
    for (const auto& h : response.entity_header)
        stream << h.first << ": " << h.second << http_constants::CRLF;

    stream << http_constants::CRLF;
    stream << response.message_body;
    return stream;
}

struct http_service_info
{
    std::string path;

#if defined(HAVE_LIBMAGIC)
    magic_set* magic_handle;
#else
    void* magic_handle;
#endif
};

#endif

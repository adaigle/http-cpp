#ifndef HTTP_STRUCTURES_HPP
#define HTTP_STRUCTURES_HPP

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <map>
#include <sstream>
#include <string>

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

    static std::string reason_phrase(uint16_t code);

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
    uint16_t    status_code;
    header_map  general_header;
    header_map  response_header;
    header_map  entity_header;
    std::string message_body;

    static http_response create_response();
};

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<< (std::basic_ostream<CharT, Traits>& stream, const http_response& response)
{
    stream << response.http_version << http_constants::SP << response.status_code << http_constants::SP << http_constants::reason_phrase(response.status_code) << http_constants::CRLF;

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

#endif

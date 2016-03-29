#ifndef HTTP_CONSTANTS_H
#define HTTP_CONSTANTS_H

#include <cstdint>
#include <ctime>
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

    static std::string http_date(std::time_t t = std::time(nullptr));

private:
    static std::string httptime(const std::tm* timeptr);
};

#endif

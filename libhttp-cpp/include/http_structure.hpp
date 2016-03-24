#ifndef HTTP_STRUCTURES_HPP
#define HTTP_STRUCTURES_HPP

#include "http_structure.h"

#include <algorithm>
#include <sstream>
#include <type_traits>

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<< (std::basic_ostream<CharT, Traits>& stream, const http_constants::method m)
{
    return stream << *(http_constants::METHODS.begin() + static_cast<std::underlying_type<http_constants::method>::type>(m));
}

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

#endif

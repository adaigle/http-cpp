///////////////////////////////////////////////////////////
// Class declaration
#include "http_constants.h"

///////////////////////////////////////////////////////////
// Other includes
#include <exception>
#include <stdexcept>

#include "logger.hpp"

constexpr decltype(http_constants::SP) http_constants::SP;
constexpr decltype(http_constants::CM) http_constants::CM;
constexpr decltype(http_constants::CRLF) http_constants::CRLF;
constexpr decltype(http_constants::METHODS) http_constants::METHODS;
constexpr decltype(http_constants::GENERAL_HEADER) http_constants::GENERAL_HEADER;
constexpr decltype(http_constants::REQUEST_HEADER) http_constants::REQUEST_HEADER;
constexpr decltype(http_constants::ENTITY_HEADER) http_constants::ENTITY_HEADER;

std::string http_constants::reason_phrase(http_constants::status code)
{
	switch (code) {
		case http_constants::status::http_unknown:
			logger::warn() << "Unknown http code." << logger::endl;
			return "Unknown";

		case http_constants::status::http_continue: 				return "Continue";
		case http_constants::status::http_switching_protocols: 		return "Switching Protocols";

		case http_constants::status::http_ok: 						return "OK";
		case http_constants::status::http_created: 					return "Created";
		case http_constants::status::http_accepted: 				return "Accepted";
		case http_constants::status::http_nonauthoritative: 		return "Non-Authoritative Information";
		case http_constants::status::http_no_content: 				return "No Content";
		case http_constants::status::http_reset_content: 			return "Reset Content";
		case http_constants::status::http_partial_content: 			return "Partial Content";

		case http_constants::status::http_multiple_choices: 		return "Multiple Choices";
		case http_constants::status::http_moved_permanently: 		return "Moved Permanently";
		case http_constants::status::http_found: 					return "Found";
		case http_constants::status::http_see_other: 				return "See Other";
		case http_constants::status::http_not_modified: 			return "Not Modified";
		case http_constants::status::http_use_proxy: 				return "Use Proxy";
		case http_constants::status::http_temporary_redirect: 		return "Temporary Redirect";

		case http_constants::status::http_bad_request: 				return "Bad Request";
		case http_constants::status::http_unhautorized: 			return "Unauthorized";
		case http_constants::status::http_payment_required: 		return "Payment Required";
		case http_constants::status::http_forbidden: 				return "Forbidden";
		case http_constants::status::http_not_found: 				return "Not Found";
		case http_constants::status::http_method_not_allowed: 		return "Method Not Allowed";
		case http_constants::status::http_not_acceptable: 			return "Not Acceptable";
		case http_constants::status::http_proxy_authentication_required: return "Proxy Authentication Required";
		case http_constants::status::http_request_timeout: 			return "Request Time-out";
		case http_constants::status::http_conflict: 				return "Conflict";
		case http_constants::status::http_gone: 					return "Gone";
		case http_constants::status::http_length_required: 			return "Length Required";
		case http_constants::status::http_precondition_failed: 		return "Precondition Failed";
		case http_constants::status::http_request_entry_too_large: 	return "Request Entity Too Large";
		case http_constants::status::http_requesturi_too_large: 	return "Request-URI Too Large";
		case http_constants::status::http_unsupported_media_type: 	return "Unsupported Media Type";
		case http_constants::status::http_requested_range_not_satisfiable: return "Requested range not satisfiable";
		case http_constants::status::http_expectation_failed: 		return "Expectation Failed";

		case http_constants::status::http_internal_server_error: 	return "Internal Server Error";
		case http_constants::status::http_not_implemented: 			return "Not Implemented";
		case http_constants::status::http_bad_gateway: 				return "Bad Gateway";
		case http_constants::status::http_service_unavailable: 		return "Service Unavailable";
		case http_constants::status::http_gateway_timeout: 			return "Gateway Time-out";
		case http_constants::status::http_version_not_supported: 	return "HTTP Version not supported";
		default: throw std::invalid_argument("The HTTP status code provided is invalid.");
	}
}

http_constants::status_class http_constants::get_status_class(http_constants::status code)
{
	const uint8_t first_digit = static_cast<std::underlying_type<decltype(code)>::type>(code) / 100;
	return static_cast<http_constants::status_class>(first_digit);
}

std::string http_constants::httptime(const std::tm* timeptr)
{
	// Construct and HTTP-date as an rfc1123-date
	static const char wday_name[][4] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static const char mon_name[][4] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

  	static char result[29];
	std::sprintf(result, "%.3s, %2d %.3s %d %.2d:%.2d:%.2d GMT",
			wday_name[timeptr->tm_wday],
			timeptr->tm_mday,
			mon_name[timeptr->tm_mon],
			1900 + timeptr->tm_year,
			timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec);
	return std::string(result);
}

std::string http_constants::http_date(std::time_t time)
{
    return httptime(localtime(&time));
}

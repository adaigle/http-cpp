#include "http_structure.hpp"

#include <algorithm>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include <tuple>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/local_time/local_time.hpp>

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

// Construct and HTTP-date as an rfc1123-date
std::string http_constants::http_date()
{
    std::stringstream date_builder;

    boost::local_time::local_date_time t(boost::local_time::local_sec_clock::local_time(boost::local_time::time_zone_ptr()));
    boost::local_time::local_time_facet* lf(new boost::local_time::local_time_facet("%a, %d %b %Y %H:%M:%S GMT"));
    date_builder.imbue(std::locale(date_builder.getloc(), lf));
    date_builder << t;

    return date_builder.str();
}

http_request::parsing_status http_request::parse(const std::string& frame)
{
	if (frame.empty())
		return parsing_status::empty_request;

	std::string request_line;
	std::istringstream input(frame);

	///////////////////////////////////////////////////////
	// Parse HTTP request
	//   Request-Line = Method SP Request-URI SP HTTP-Version
	std::getline(input, request_line);

	std::vector<std::string> request_line_split;
	boost::split(request_line_split, request_line, boost::is_space());
	if (request_line_split.size() > 0 && request_line_split.back().length() == 0) {
		request_line_split.pop_back();
	}

	if (request_line_split.size() != 3)
		return parsing_status::invalid_request_line;

	const std::string raw_method = request_line_split[0];
	const std::string raw_request_uri = request_line_split[1];
	const std::string raw_http_version = request_line_split[2];

	const auto find_iter =
	    std::find(std::cbegin(http_constants::METHODS), std::cend(http_constants::METHODS), raw_method);
	if (find_iter == std::cend(http_constants::METHODS))
		return parsing_status::invalid_request_line;
	const size_t find_index = std::distance(std::cbegin(http_constants::METHODS), find_iter);

	method = static_cast<http_constants::method>(find_index);
	request_uri = raw_request_uri;
	http_version = raw_http_version;

	///////////////////////////////////////////////////////
	// Parse HTTP headers
	//   *(( general-header | request-header | entity-header ) CRLF) CRLF
	for (std::string line; std::getline(input, line) && !line.empty();) {
		const size_t colon_position = line.find_first_of(":");
		if (colon_position == std::string::npos)
			continue;

		std::string header(line.substr(0, colon_position));
		std::string property(line.substr(colon_position + 1));

		const auto general_iter =
		    std::find(std::cbegin(http_constants::GENERAL_HEADER), std::cend(http_constants::GENERAL_HEADER), header);
		const auto request_iter =
		    std::find(std::cbegin(http_constants::REQUEST_HEADER), std::cend(http_constants::REQUEST_HEADER), header);
		const auto entity_iter =
		    std::find(std::cbegin(http_constants::ENTITY_HEADER), std::cend(http_constants::ENTITY_HEADER), header);

		if (general_iter != std::cend(http_constants::GENERAL_HEADER)) {
			general_header.emplace(header, property);
		}
		else if (general_iter != std::cend(http_constants::REQUEST_HEADER)) {
			request_header.emplace(header, property);
		}
		else {
			entity_header.emplace(header, property);
		}
	}

	///////////////////////////////////////////////////////
	// Parse message body
	message_body = "";
	if (input.tellg() < frame.size())
		message_body = frame.substr(input.tellg());

	///////////////////////////////////////////////////////
	// Print out the request to the console

	std::string printable_request_line = request_line;
	boost::replace_all(printable_request_line, "\r", "" /*"\\r"*/);
	logger::info() << printable_request_line << logger::endl;
	for (const auto& h : general_header)
		logger::wire() << h.first << ":" << h.second << logger::endl;
	for (const auto& h : request_header)
		logger::wire() << h.first << ":" << h.second << logger::endl;
	for (const auto& h : entity_header)
		logger::wire() << h.first << ":" << h.second << logger::endl;

	return parsing_status::success;
}

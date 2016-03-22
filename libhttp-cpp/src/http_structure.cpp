#include "../include/http_structure.hpp"

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

std::string http_constants::reason_phrase(uint16_t code)
{
	switch (code) {
		case 100: return "Continue";
		case 101: return "Switching Protocols";

		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 203: return "Non-Authoritative Information";
		case 204: return "No Content";
		case 205: return "Reset Content";
		case 206: return "Partial Content";

		case 300: return "Multiple Choices";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 305: return "Use Proxy";
		case 307: return "Temporary Redirect";

		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 402: return "Payment Required";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 407: return "Proxy Authentication Required";
		case 408: return "Request Time-out";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 412: return "Precondition Failed";
		case 413: return "Request Entity Too Large";
		case 414: return "Request-URI Too Large";
		case 415: return "Unsupported Media Type";
		case 416: return "Requested range not satisfiable";
		case 417: return "Expectation Failed";

		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Time-out";
		case 505: return "HTTP Version not supported";
		default: throw std::invalid_argument("The HTTP status code provided is invalid.");
	}
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

http_response http_response::create_response()
{
	http_response response;

	// Default HTTP version, potentially overwritten on request.
	response.http_version = "HTTP/1.1";

    response.response_header["Server"] = "http-cpp v0.1";
    response.general_header["Date"] = http_constants::http_date();

	return response;
}

#include "http_protocol_one_one.h"

#include <cassert>
#include <exception>
#include <sstream>
#include <stdexcept>

#include "interface/http_resource.h"
#include "http_resource_factory.h"
#include "http_structure.hpp"

#include <boost/algorithm/string.hpp>

#include "logger.h"

constexpr decltype(http_protocol_one_one::http_version) http_protocol_one_one::http_version;

http_protocol_one_one::http_protocol_one_one() noexcept :
    http_protocol_handler()
{
    // Add supported http method.
    dispatcher_.emplace(http_constants::method::m_get, &http_protocol_one_one::execute_get);
    dispatcher_.emplace(http_constants::method::m_head, &http_protocol_one_one::execute_head);
}


http_request::parsing_status http_protocol_one_one::parse_request(const std::string& request, http_request& structured_request) const noexcept
{
	if (request.empty())
		return http_request::parsing_status::empty_request;

	std::string request_line;
	std::istringstream input(request);

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
		return http_request::parsing_status::invalid_request_line;

	const std::string raw_method = request_line_split[0];
	const std::string raw_request_uri = request_line_split[1];
	const std::string raw_http_version = request_line_split[2];

	const auto find_iter =
	    std::find(std::cbegin(http_constants::METHODS), std::cend(http_constants::METHODS), raw_method);
	if (find_iter == std::cend(http_constants::METHODS))
		return http_request::parsing_status::invalid_request_line;
	const size_t find_index = std::distance(std::cbegin(http_constants::METHODS), find_iter);

	structured_request.method = static_cast<http_constants::method>(find_index);
	structured_request.request_uri = raw_request_uri;
	structured_request.http_version = raw_http_version;

	///////////////////////////////////////////////////////
	// Parse HTTP headers
	//   *(( general-header | request-header | entity-header ) CRLF) CRLF
	for (std::string line; std::getline(input, line) && !line.empty();) {
		const size_t colon_position = line.find_first_of(":");
		if (colon_position == std::string::npos)
			continue;
		if (line.length() < 2)
			continue;

		if (line.back() == '\n') line.pop_back();
		if (line.back() == '\r') line.pop_back();

		std::string header(line.substr(0, colon_position));
		std::string property(line.substr(colon_position + 2));

		const auto general_iter =
		    std::find(std::cbegin(http_constants::GENERAL_HEADER), std::cend(http_constants::GENERAL_HEADER), header);
		const auto request_iter =
		    std::find(std::cbegin(http_constants::REQUEST_HEADER), std::cend(http_constants::REQUEST_HEADER), header);
		const auto entity_iter =
		    std::find(std::cbegin(http_constants::ENTITY_HEADER), std::cend(http_constants::ENTITY_HEADER), header);

		if (general_iter != std::cend(http_constants::GENERAL_HEADER)) {
			structured_request.general_header.emplace(header, property);
		} else if (request_iter != std::cend(http_constants::REQUEST_HEADER)) {
			structured_request.request_header.emplace(header, property);
		} else if (entity_iter  != std::cend(http_constants::REQUEST_HEADER)) {
			structured_request.entity_header.emplace(header, property);
		} else {
			assert(false);
		}

	}

	///////////////////////////////////////////////////////
	// Parse message body
	structured_request.message_body = "";
	if (input.tellg() < request.size())
		structured_request.message_body = request.substr(input.tellg());

	///////////////////////////////////////////////////////
	// Print out the request to the console

	std::string printable_request_line = request_line;
	boost::replace_all(printable_request_line, "\r", "" /*"\\r"*/);
	logger::log()->trace() << printable_request_line;
	for (const auto& h : structured_request.general_header)
		logger::log()->trace() << h.first << ":" << h.second;
	for (const auto& h : structured_request.request_header)
		logger::log()->trace() << h.first << ":" << h.second;
	for (const auto& h : structured_request.entity_header)
		logger::log()->trace() << h.first << ":" << h.second;

	return http_request::parsing_status::success;
}

http_response http_protocol_one_one::make_response(const generic_response& gresponse) const noexcept
{
    http_response response(gresponse, http_version);

    for (const auto& header_value : gresponse.header) {
        // TODO: Dispatch header based on key.
        response.response_header[header_value.first] = header_value.second;
    }

    response.response_header["Server"] = "http-cpp v0.1";
    response.general_header["Date"] = http_constants::http_date();
    return response;
}

//void http_protocol_one_one::execute(const http_resource_factory* const resource_factory, const http_request& request, http_response& response)
//{
//    const auto it = dispatcher_.find(request.method);
//    if (it != dispatcher_.cend()) {
//        try {
//            std::unique_ptr<http_resource> resource = resource_factory->create_handle(request);
//            if (!resource) {
//                response.status_code = http_constants::status::http_not_found;
//                return;
//            }
//
//            resource->execute(request, response);
//        } catch (std::exception& e) {
//            // Resource cannot be loaded, send out a 404 response.
//            response.status_code = http_constants::status::http_not_found;
//        }
//    } else {
//        response.status_code = http_constants::status::http_not_implemented;
//        response.entity_header["Allow"] = list_implemented_methods();
//    }
//}
//
//void http_protocol_one_one::execute_get(const http_resource_factory* const resource_factory, const http_request& request, http_response& response) const
//{
//    try {
//        std::unique_ptr<http_resource> resource = resource_factory->create_handle(request);
//        if (!resource) {
//            response.status_code = http_constants::status::http_not_found;
//            return;
//        }
//
//        resource->execute(request, response);
//
//        /*const auto header = resource->fetch_resource_header();
//        response.entity_header["Content-Type"] = header.at("Content-Type");
//        response.entity_header["Content-Length"] = header.at("Content-Length");
//        response.entity_header["Last-Modified"] = header.at("Last-Modified");
//
//        std::ostringstream resource_stream;
//        resource->fetch_resource_content(resource_stream);
//        response.message_body = resource_stream.str();*/
//
//        response.status_code = http_constants::status::http_ok;
//    } catch (std::exception& e) {
//        // Resource cannot be loaded, send out a 404 response.
//        response.status_code = http_constants::status::http_not_found;
//    }
//}
//
//void http_protocol_one_one::execute_head(const http_resource_factory* const resource_factory, const http_request& request, http_response& response) const
//{
//    try {
//        std::unique_ptr<http_resource> resource = resource_factory->create_handle(request);
//        if (!resource) {
//            response.status_code = http_constants::status::http_not_found;
//            return;
//        }
//
//        resource->execute(request, response);
//
//        /*const auto header = resource->fetch_resource_header();
//        response.entity_header["Content-Type"] = header.at("Content-Type");
//        response.entity_header["Content-Length"] = header.at("Content-Length");
//        response.entity_header["Last-Modified"] = header.at("Last-Modified");*/
//
//        response.status_code = http_constants::status::http_ok;
//    } catch (std::exception& e) {
//        // Resource cannot be loaded, send out a 404 response.
//        response.status_code = http_constants::status::http_not_found;
//    }
//}

std::string http_protocol_one_one::list_implemented_methods() const
{
    std::ostringstream ss;
    bool first = true;
    for (const auto& m : dispatcher_) {
        if (!first) ss << ", ";
        else first = false;
        ss << m.first;
    }
    return ss.str();
}

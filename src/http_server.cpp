#include "http_server.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include <boost/filesystem.hpp>

#include "logger.hpp"

http_server::http_server(uint8_t io_threads) :
    context(io_threads), http_sockets()
{
}

void http_server::connect(uint16_t port, const std::string& website_root, const std::string& website_name)
{
    logger::trace() << "Connecting to port " << port << " for website '" << website_name << "'..." << logger::endl;

    // Check if the port is already used.
    const auto find_it = std::find_if(std::cbegin(http_sockets), std::cend(http_sockets),
                                        [&port](const socket_info& info) { return info.port == port; });
    if (find_it != http_sockets.cend())
        throw std::invalid_argument("The binding of the socket was unsuccessful. Did you register two websites under the same port ?");

    // Check if the website directory exists.
    if (!boost::filesystem::is_directory(website_root))
        throw std::invalid_argument("Invalid website root directory. The directory must exist on the filesystem.");

    const auto insert_iter = websites.emplace(port, website_root, website_name);
    if (!insert_iter.second) {
        throw std::invalid_argument("Invalid website identifier. Is the port and name combination already used ?");
    }

    auto socket_it = http_sockets.insert(http_sockets.end(),
        socket_info(port, std::make_unique<zmq::socket_t>(context, zmq::socket_type::stream)));
    try {
        // Executing the binding.
        std::ostringstream address_builder;
        address_builder << "tcp://*:" << port;
        socket_it->socket->bind(address_builder.str());
    } catch (...) {
        websites.erase(insert_iter.first);
        http_sockets.erase(socket_it);
        throw;
    }
}

void http_server::run()
{
    logger::info() << "Server ready." << logger::endl;

    // Initialize the lists of sockets to poll from
    std::vector<zmq::pollitem_t> poll_items;
    poll_items.reserve(http_sockets.size());
    for (socket_info& info : http_sockets) {
        poll_items.emplace_back(zmq::pollitem_t{static_cast<void*>(*info.socket), 0, ZMQ_POLLIN, 0});
    }

    while (true) {
        // Extract identity from the request.

        size_t i;
        zmq::poll(poll_items, -1);
        for (i = 0; i < poll_items.size() && !(poll_items[i].revents & ZMQ_POLLIN); ++i);

        if (i == poll_items.size())
            continue;

        identity_t id;
        id.length = http_sockets[i].socket->recv(&id.identity, id.identity.size());

        // Construct a transaction for the request.
        http_transaction_t transaction(std::move(id));
        logger::trace() << "Transaction initiated with id '" << transaction.id << "'." << logger::endl;

        if (!extract_request(*http_sockets[i].socket, transaction.request))
            continue; // Ignore the current request if the receiving fails...

        // Find the website to assign the request to...

        // Create response.
        transaction.response.http_version = transaction.request.http_version;

        {
            const std::string host = extract_host(transaction.request);
            const virtual_website& website = find_website(http_sockets[i].port, transaction.request);

            const auto& host_it = transaction.request.request_header.find("Host");
            const auto& header_end = transaction.request.request_header.cend();
            const bool success = website.lookup_ressource(transaction.request.request_uri,
                                                          (host_it != header_end) ? host_it->second : "",
                                                          transaction.response);

            if (!success) {
                transaction.response.status_code = 404;
            } else {
                transaction.response.status_code = 200;
            }
        }

        if (!send_response(*http_sockets[i].socket, transaction))
            continue; // Ignore the current request if the receiving fails...
    }
}

bool http_server::extract_request(zmq::socket_t& socket, http_request& request)
{
    std::string frame;

    // Extract the request itself.
    int64_t more;
    do {
        zmq::message_t part;
        socket.recv(&part);

        frame.append(static_cast<char*>(part.data()), part.size());

        size_t more_size = sizeof(more);
        socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
    } while (more);

    // Parse the request
    http_request::parsing_status code = request.parse(frame);
    if (code != http_request::parsing_status::success) {
        if (code == http_request::parsing_status::empty_request) {
            logger::trace() << "Dropping empty request." << logger::endl;
        } else {
            logger::warn() << "parsing error:" << static_cast<size_t>(code) << "." << logger::endl;
        }
    }
    return code == http_request::parsing_status::success;
}

bool http_server::send_response(zmq::socket_t& socket, http_transaction_t& t)
{
    //std::cout << "RESPONSE" << std::endl;
    //std::cout << t.response.content << std::endl;
    //std::cout << std::endl;

    std::ostringstream response_builder;
    response_builder << t.response;
    const std::string response = response_builder.str();

    socket.send(&t.id.identity, t.id.length, ZMQ_SNDMORE);
    zmq::message_t wire_response(response.c_str(), response.size());
    socket.send(wire_response, ZMQ_SNDMORE);

    // Close the connection with an empty response.
    socket.send(&t.id.identity, t.id.length, ZMQ_SNDMORE);
    socket.send(nullptr, 0, ZMQ_SNDMORE);
    // Keep using ZMQ_SNDMORE to let another client connects.

    const uint8_t first_digit = t.response.status_code / 100;
    logger::type response_type;
    switch (first_digit) {
        case 1: case 3: response_type = logger::type::info; break;
        case 2: response_type = logger::type::info_green; break;
        case 4: response_type = logger::type::warning; break;
        case 5: response_type = logger::type::error; break;
        default: response_type = logger::type::error;
    }

    if (logger::check_log(response_type)) {
        std::string line;
        std::istringstream input(response);
        std::getline(input, line);
        logger::log(response_type) << line << logger::endl;
    }

    return true;
}

std::string http_server::extract_host(http_request& request)
{
    std::string host = request.request_header["Host"];

    // Check if the Request-URI is an absoluteURI of the following form:
    // "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
    std::regex absolute_uri_regex("http:\\/\\/([^\\/:]+)(:([0-9]+))?([^\\?]*)(\\?(.*))?");

    std::smatch matches;
    if (std::regex_match(request.request_uri, matches, absolute_uri_regex)) {
        // In this case, RFC2616:5.2 says to ignore any host header.
        // We rewrite it in the request_header field.

        const std::string raw_host = matches[1];
        const std::string raw_port = matches[3];
        const std::string raw_abs_path = matches[4];
        const std::string raw_query = matches[6];

        logger::trace() << "host: " << raw_host << logger::endl;
        logger::trace() << "port: " << raw_port << logger::endl;
        logger::trace() << "abs_path: " << raw_abs_path << logger::endl;
        logger::trace() << "query: " << raw_query << logger::endl;

        host = request.request_header["Host"] = raw_host;
    }
    logger::trace() << "not an absolute uri..." << logger::endl;

    return host;
}

const virtual_website& http_server::find_website(uint16_t port, http_request& request) const
{
    auto iter = std::find_if(std::cbegin(websites), std::cend(websites),
                             [&port](const virtual_website& w) { return w.port == port; });
    return *iter;
}

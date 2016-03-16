#include "http_worker.h"

#include <chrono>
#include <exception>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/date_time/local_time/local_time.hpp>

#include "logger.hpp"

using namespace std::chrono_literals;

http_worker::http_worker(zmq::context_t& context, size_t id, const std::set<virtual_website>& ws) :
    main_context(context), worker_id(id), websites(ws)
{
}

void http_worker::run()
{
    zmq::socket_t inproc_status_socket(main_context, zmq::socket_type::sub);
    zmq::socket_t inproc_request_socket(main_context, zmq::socket_type::dealer);
    try {
        inproc_status_socket.connect("inproc://http_workers_status");
        // Subscribe on everything
        inproc_status_socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    } catch (std::exception& e) {
        logger::error() << "Server error, cannot connect the HTTP worker status channel: " << logger::endl;
        logger::error() << e.what() << logger::endl;
        throw e;
    }
    try {
        inproc_request_socket.connect("inproc://http_workers_requests");
    } catch (std::exception& e) {
        logger::error() << "Server error, cannot connect the HTTP worker request channel: " << logger::endl;
        logger::error() << e.what() << logger::endl;
        throw e;
    }

    logger::info() << "Worker #" << worker_id << " online." << logger::endl;

    // Initialize the lists of sockets to poll from
    std::vector<zmq::pollitem_t> poll_items = {
        zmq::pollitem_t{static_cast<void*>(inproc_status_socket), 0, ZMQ_POLLIN, 0},
        zmq::pollitem_t{static_cast<void*>(inproc_request_socket), 0, ZMQ_POLLIN, 0}
    };

    try {
        while (running) {
                zmq::poll(poll_items, -1);

                // Handle status message in priority
                if (poll_items[0].revents & ZMQ_POLLIN) {
                    handle_status(inproc_status_socket);
                }
                // Handle status message in priority
                if (poll_items[1].revents & ZMQ_POLLIN) {
                    handle_request(inproc_request_socket);
                }
        }
    } catch (std::exception& e) {
        logger::error() << "Exception caught in worker thread #" << worker_id << ":" << logger::endl;
        logger::error() << e.what() << logger::endl;
        throw e;
    }

    logger::info() << "Worker #" << worker_id << " shut down." << logger::endl;
}

void http_worker::handle_status(zmq::socket_t& socket)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(worker_id * 10));

    logger::debug() << "Worker #" << worker_id << ": receiving status code..." << logger::endl;
    zmq::message_t status_message;
    socket.recv(&status_message);

    std::string status_str(static_cast<char*>(status_message.data()), status_message.size());
    logger::info() << "Worker #" << worker_id << ": " << status_str << logger::endl;

    if (status_str == "QUIT") {
        logger::debug() << "Closing worker #" << worker_id << "..." << logger::endl;
        running = false;
    }
}

void http_worker::handle_request(zmq::socket_t& socket)
{
    identity_t id;
    id.length = socket.recv(&id.identity, id.identity.size());

    // Construct a transaction for the request.
    http_transaction_t transaction(std::move(id));
    logger::debug() << "Worker #" << worker_id << ": processing transaction '" << transaction.id << "'." << logger::endl;
    try {
        ///////////////////////////////////////////////////
        // 1. Extract the request from the inproc messaging queue.

        if (!extract_request(socket, transaction.request)) {
            // Ignore the current request if the receiving fails...
            transaction.response.status_code = 400;
        } else {

            ///////////////////////////////////////////////////
            // 2. Detect the website based on the host/port of the requets-URI.

            const std::string host = extract_host(transaction.request);
            const virtual_website& website = find_website(8081, transaction.request);

            ///////////////////////////////////////////////////
            // 3. Load the appropriate ressource from the website.

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
    } catch(...) {
        transaction.response.status_code = 500;
    }

    ///////////////////////////////////////////////////
    // 4. Construct the response and complete the request.

    transaction.response.http_version = transaction.request.http_version;
    transaction.response.response_header["Server"] = "http-cpp v0.1";
    transaction.response.general_header["Date"] = http_date();

    if (!send_response(socket, transaction))
        return; // Ignore the current request if the receiving fails...
}



bool http_worker::extract_request(zmq::socket_t& socket, http_request& request)
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

bool http_worker::send_response(zmq::socket_t& socket, http_transaction_t& t)
{
    std::ostringstream response_builder;
    response_builder << t.response;
    const std::string response = response_builder.str();

    socket.send(&t.id.identity, t.id.length, ZMQ_SNDMORE);
    zmq::message_t wire_response(response.c_str(), response.size());
    socket.send(wire_response);

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

// Construct and HTTP-date as an rfc1123-date
std::string http_worker::http_date()
{
    std::stringstream date_builder;

    boost::local_time::local_date_time t(boost::local_time::local_sec_clock::local_time(boost::local_time::time_zone_ptr()));
    boost::local_time::local_time_facet* lf(new boost::local_time::local_time_facet("%a, %d %b %Y %H:%M:%S GMT"));
    date_builder.imbue(std::locale(date_builder.getloc(), lf));
    date_builder << t;

    return date_builder.str();
}

std::string http_worker::extract_host(http_request& request)
{
    std::string host = request.request_header["Host"];

    // Check if the Request-URI is an absoluteURI of the following form:
    // "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
    static std::regex absolute_uri_regex("http:\\/\\/([^\\/:]+)(:([0-9]+))?([^\\?]*)(\\?(.*))?", std::regex_constants::ECMAScript | std::regex_constants::optimize);
    static std::regex absolute_path_regex("(\\/([^\\?]*))(\\?(.*))?", std::regex_constants::ECMAScript | std::regex_constants::optimize);

    std::smatch matches;
    if (std::regex_match(request.request_uri, matches, absolute_uri_regex)) {

        // In this case, RFC2616:5.2 says to ignore any host header.
        // We rewrite it in the request_header field.

        const std::string raw_host = matches[1];
        const std::string raw_port = matches[3];
        const std::string raw_abs_path = matches[4];
        const std::string raw_query = matches[6];

        logger::trace() << "Absolute URI detected: " << logger::endl;
        logger::trace() << " - Host: " << raw_host << "(" << raw_port << ")" << logger::endl;
        logger::trace() << " - Abs_path: " << raw_abs_path << logger::endl;
        logger::trace() << " - Query: " << raw_query << logger::endl;

        request.request_header["Host"] = raw_host;
        return raw_host;
    }

    if (std::regex_match(request.request_uri, matches, absolute_path_regex)) {
        const std::string raw_abs_path = matches[1];
        const std::string raw_query = matches[4];

        logger::trace() << "Absolute path detected: " << logger::endl;
        logger::trace() << " - Abs_path: " << raw_abs_path << logger::endl;
        logger::trace() << " - Query: " << raw_query << logger::endl;

        return request.request_header["Host"];
    }

    if (request.request_uri == "*") {
        // Nothing to do.
        return "*";
    }

    // TODO: Handle 'authority'-based request.

    logger::warn() << "Invalid Request-URI: " << request.request_uri << logger::endl;
    throw std::invalid_argument("Invalid Request-URI parameter in the request, could not detect a valid format.");
}

const virtual_website& http_worker::find_website(uint16_t port, http_request& request) const
{
    auto iter = std::find_if(std::cbegin(websites), std::cend(websites),
                             [&port](const virtual_website& w) { return w.port == port; });
    return *iter;
}

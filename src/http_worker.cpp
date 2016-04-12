#include "http_worker.h"

#include <chrono>
#include <exception>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/date_time/local_time/local_time.hpp>

#include "http_exception.h"
#include "http_structure.hpp"

#include "logger.hpp"
#include "identity.h"

using namespace std::chrono_literals;

http_worker::http_worker(zmq::context_t& context, size_t id, const std::set<http_website>& ws) :
    main_context_(context), identifier_(id), websites_(ws)
{
}

void http_worker::run()
{
    zmq::socket_t inproc_status_socket(main_context_, zmq::socket_type::sub);
    zmq::socket_t inproc_request_socket(main_context_, zmq::socket_type::dealer);
    try {
        inproc_status_socket.connect("inproc://http_workers_status");
        // Subscribe on everything
        inproc_status_socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    } catch (zmq::error_t& e) {
        logger::error() << "Server error, cannot connect the HTTP worker status channel: " << logger::endl;
        logger::error() << "Error " << zmq_errno() << ": " << e.what() << logger::endl;
        throw e;
    }
    try {
        inproc_request_socket.connect("inproc://http_workers_requests");
    } catch (zmq::error_t& e) {
        logger::error() << "Server error, cannot connect the HTTP worker request channel: " << logger::endl;
        logger::error() << "Error " << zmq_errno() << ": " << e.what() << logger::endl;
        throw e;
    }

    logger::info() << "Worker #" << identifier_ << " online." << logger::endl;

    // Initialize the lists of sockets to poll from
    std::vector<zmq::pollitem_t> poll_items = {
        zmq::pollitem_t{static_cast<void*>(inproc_status_socket), 0, ZMQ_POLLIN, 0},
        zmq::pollitem_t{static_cast<void*>(inproc_request_socket), 0, ZMQ_POLLIN, 0}
    };

    while (running) {
        try {
            zmq::poll(poll_items, -1);

            // Handle status message in priority
            if (poll_items[0].revents & ZMQ_POLLIN) {
                handle_status(inproc_status_socket);
            }
            // Handle status message in priority
            if (poll_items[1].revents & ZMQ_POLLIN) {
                handle_request(inproc_request_socket);
            }
        } catch (zmq::error_t& e) {
            logger::error() << "Exception caught in worker thread #" << identifier_ << ":" << logger::endl;
            logger::error() << "Error " << zmq_errno() << ": " << e.what() << logger::endl;
        }
    }

    logger::info() << "Worker #" << identifier_ << " shut down." << logger::endl;
}

void http_worker::handle_status(zmq::socket_t& socket)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(identifier_ * 10));

    logger::debug() << "Worker #" << identifier_ << ": receiving status code..." << logger::endl;
    zmq::message_t status_message;
    socket.recv(&status_message);

    std::string status_str(static_cast<char*>(status_message.data()), status_message.size());
    logger::info() << "Worker #" << identifier_ << ": " << status_str << logger::endl;

    if (status_str == "QUIT") {
        logger::debug() << "Closing worker #" << identifier_ << "..." << logger::endl;
        running = false;
    }
}

void http_worker::handle_request(zmq::socket_t& socket)
{
    identity_t id;
    id.length = socket.recv(&id.identity, id.identity.size());

    // Construct a transaction for the request.
    logger::debug() << "Worker #" << identifier_ << ": processing transaction '" << id << "'." << logger::endl;

    http_response response;
    try {
        ///////////////////////////////////////////////////
        // 1. Extract the request from the inproc messaging queue.

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

        ///////////////////////////////////////////////////
        // 2. Detect the website based on the host/port of the requets-URI.
        const http_request request = http_service::parse_request(frame);
        const http_website& website = find_website(request);

        logger::info() << "website: " << website.host() << logger::endl;


        ///////////////////////////////////////////////////
        // 3. Execute the http request.
        response = website.execute(request);
    } catch(http_invalid_request& e) {
        response.status_code = http_constants::status::http_bad_request;
    } catch(...) {
        response.status_code = http_constants::status::http_internal_server_error;
    }

    ///////////////////////////////////////////////////
    // 4. Construct the response and complete the request.

    std::ostringstream response_builder;
    response_builder << response;
    const std::string string_response = response_builder.str();

    socket.send(&id.identity, id.length, ZMQ_SNDMORE);
    socket.send(string_response.c_str(), string_response.size());

    logger::type response_type;
    switch (http_constants::get_status_class(response.status_code)) {
        case http_constants::status_class::informational:
        case http_constants::status_class::redirection: response_type = logger::type::info; break;
        case http_constants::status_class::success: response_type = logger::type::info_green; break;
        case http_constants::status_class::client_error: response_type = logger::type::warning; break;
        case http_constants::status_class::server_error: response_type = logger::type::error; break;
        default: response_type = logger::type::error;
    }

    if (logger::check_log(response_type)) {
        std::string line;
        std::istringstream input(string_response);
        std::getline(input, line);
        logger::log(response_type) << line << logger::endl;
    }
}

const http_website& http_worker::find_website(const http_request& request) const
{
    const http_service::host host = http_service::extract_host(request);

    auto iter = std::find(std::cbegin(websites_), std::cend(websites_), host);
    if (iter == std::cend(websites_)) {
        logger::warn() << "Unknown website '" << host << "' among: " << logger::endl;
        for (const auto& website : websites_)
            logger::warn() << website.host() << logger::endl;
        throw http_invalid_request("Unknown website...");
    }

    return *iter;
}

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

#include "logger.h"
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
        logger::log(logger::type::worker)->error() << "Server error, cannot connect the HTTP worker status channel: ";
        logger::log(logger::type::worker)->error() << "Error " << zmq_errno() << ": " << e.what();
        throw e;
    }
    try {
        inproc_request_socket.connect("inproc://http_workers_requests");
    } catch (zmq::error_t& e) {
        logger::log(logger::type::worker)->error() << "Server error, cannot connect the HTTP worker request channel: ";
        logger::log(logger::type::worker)->error() << "Error " << zmq_errno() << ": " << e.what();
        throw e;
    }

    logger::log(logger::type::worker)->info() << "Worker #" << identifier_ << " online.";

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
            logger::log(logger::type::worker)->error() << "Exception caught in worker thread #" << identifier_ << ":";
            logger::log(logger::type::worker)->error() << "Error " << zmq_errno() << ": " << e.what();
        }
    }

    logger::log(logger::type::worker)->info() << "Worker #" << identifier_ << " shut down.";
}

void http_worker::handle_status(zmq::socket_t& socket)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(identifier_ * 10));

    logger::log(logger::type::worker)->debug() << "Worker #" << identifier_ << ": receiving status code...";
    zmq::message_t status_message;
    socket.recv(&status_message);

    std::string status_str(static_cast<char*>(status_message.data()), status_message.size());
    logger::log(logger::type::worker)->info() << "Worker #" << identifier_ << ": " << status_str;

    if (status_str == "QUIT") {
        logger::log(logger::type::worker)->debug() << "Closing worker #" << identifier_ << "...";
        running = false;
    }
}

void http_worker::handle_request(zmq::socket_t& socket)
{
    identity_t id;
    id.length = socket.recv(&id.identity, id.identity.size());

    // Construct a transaction for the request.
    logger::log(logger::type::worker)->debug() << "Worker #" << identifier_ << ": processing transaction '" << id << "'.";

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

        logger::log(logger::type::worker)->info() << website.host() << " '" << request.method << " " << request.request_uri << " " << request.http_version << "'";

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

    std::string line;
    std::istringstream input(string_response);
    std::getline(input, line);
    switch (http_constants::get_status_class(response.status_code)) {
        case http_constants::status_class::informational:
        case http_constants::status_class::redirection:
            logger::log(logger::type::worker)->notice() << line; break;
        case http_constants::status_class::success:
            logger::log(logger::type::worker)->info() << line; break;
        case http_constants::status_class::client_error:
            logger::log(logger::type::worker)->warn() << line; break;
        case http_constants::status_class::server_error:
        default:
            logger::log(logger::type::worker)->error() << line; break;
    }
}

const http_website& http_worker::find_website(const http_request& request) const
{
    const http_service::host host = http_service::extract_host(request);

    auto iter = std::find(std::cbegin(websites_), std::cend(websites_), host);
    if (iter == std::cend(websites_)) {
        logger::log(logger::type::worker)->warn() << "Unknown website '" << host << "' among: ";
        for (const auto& website : websites_)
            logger::log(logger::type::worker)->warn() << website.host();
        throw http_invalid_request("Unknown website...");
    }

    return *iter;
}

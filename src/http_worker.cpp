#include "http_worker.h"

#include <chrono>
#include <exception>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/date_time/local_time/local_time.hpp>

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

    logger::info() << "Worker #" << identifier_ << " online." << logger::endl;

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
        logger::error() << "Exception caught in worker thread #" << identifier_ << ":" << logger::endl;
        logger::error() << e.what() << logger::endl;
        throw e;
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
        http_request request = http_service::parse_request(frame);
        const http_website& website = find_website(8081, request);

        ///////////////////////////////////////////////////
        // 3. Execute the http request.
        response = website.execute(request);
    } catch(...) {
        response.status_code = 500;
    }

    ///////////////////////////////////////////////////
    // 4. Construct the response and complete the request.

    std::ostringstream response_builder;
    response_builder << response;
    const std::string string_response = response_builder.str();

    socket.send(&id.identity, id.length, ZMQ_SNDMORE);
    socket.send(string_response.c_str(), string_response.size());

    const uint8_t first_digit = response.status_code / 100;
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
        std::istringstream input(string_response);
        std::getline(input, line);
        logger::log(response_type) << line << logger::endl;
    }
}

const http_website& http_worker::find_website(uint16_t port, http_request& request) const
{
    auto iter = std::find_if(std::cbegin(websites_), std::cend(websites_),
                             [&port](const http_website& w) { return w.port_ == port; });
    return *iter;
}

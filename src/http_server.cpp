#include "http_server.h"

#include <cassert>
#include <cerrno>
#include <chrono>
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
#include <spdlog/spdlog.h>

#include "http_structure.hpp"
#include "identity.h"

using namespace std::chrono_literals;

http_server::http_server(uint8_t io_threads) :
    context_(io_threads), http_socket_(context_, zmq::socket_type::stream),
    inproc_status_socket_(context_, zmq::socket_type::pub), inproc_request_socket_(context_, zmq::socket_type::dealer)
{
    logger_ = spdlog::get("server");

    try {
        inproc_status_socket_.bind("inproc://http_workers_status");
    } catch (zmq::error_t& e) {
        logger_.error() << "Server error, cannot bind the HTTP worker status channel: ";
        logger_.error() << "Error " << zmq_errno() << ": " << e.what();
        throw e;
    }
    try {
        inproc_request_socket_.bind("inproc://http_workers_requests");
    } catch (zmq::error_t& e) {
        logger_.error() << "Server error, cannot bind the HTTP worker request channel: ";
        logger_.error() << "Error " << zmq_errno() << ": " << e.what();
        throw e;
    }
}

void http_server::connect(const std::string& website_path, const std::string& host_name,
                          const uint16_t port /* = 80 */, const std::string& website_name /* = "" */)
{
    logger_.trace() << "Connecting to port " << port << " with hostname '" << host_name << "'...";

    // TODO: Check if the port is already used.

    // Check if the website directory exists.
    if (!boost::filesystem::exists(website_path))
        throw std::invalid_argument("Invalid website root directory. The directory must exist on the filesystem.");

    try {
        const auto insert_iter = websites_.emplace(website_path, http_service::host{host_name, port}, website_name);
        if (!insert_iter.second) {
            throw std::invalid_argument("Invalid website identifier. Is the port and name combination already used ?");
        }

        try {
            // Executing the binding.
            std::ostringstream address_builder;
            address_builder << "tcp://*:" << port;
            http_socket_.bind(address_builder.str());
        } catch (std::exception& e) {
            websites_.erase(insert_iter.first);
            throw;
        }
    } catch (zmq::error_t& e) {
        logger_->error() << "Server error, cannot connect to website '" << website_name << "' on port " << port << ".";
        logger_->error() << "Error " << zmq_errno() << ": " << e.what();
        throw;
    }
}

void http_server::run()
{
    // Launch the worker threads...
    logger_->debug() << "Launching worker thread...";
    for (size_t i = 0; i < 4; ++i) {
        workers_.emplace_front(context_, i, websites_);
        workers_.front().start();
    }

    logger_->debug() << "Sending start signal...";
    const std::string start = "START";
    zmq::message_t start_message(start.c_str(), start.size());
    inproc_status_socket_.send(start_message);

    std::this_thread::sleep_for(500ms);

    logger_->notice() << "Server ready.";

    // zmq_proxy doesn't work for stream sockets (identity is not sent along with the message to the same service).
    // We do the polling loop manually...

    //  Initialize poll set
    std::vector<zmq::pollitem_t> poll_items = {
        zmq::pollitem_t{static_cast<void*>(http_socket_), 0, ZMQ_POLLIN, 0},
        zmq::pollitem_t{static_cast<void*>(inproc_request_socket_),  0, ZMQ_POLLIN, 0}
    };

    try {
        while (true) {
            zmq::poll(poll_items, -1);

            if (poll_items[0].revents & ZMQ_POLLIN) {
                ///////////////////////////////////////////////
                // Forward incoming HTTP request to a worker.
                forward_as_req(http_socket_, inproc_request_socket_);
            }

            if (poll_items[1].revents & ZMQ_POLLIN) {
                ///////////////////////////////////////////////
                // Forward the HTTP response back to the client.
                forward_as_stream(inproc_request_socket_, http_socket_);
            }
        }
    } catch (zmq::error_t& e) {
        logger_->error() << "Server error, proxy failed due to the following zmq exception: ";
        logger_->error() << "Error " << zmq_errno() << ": " << e.what();
        throw e;
    }

    logger_->debug() << "Shutting down server...";

    const std::string quit = "QUIT";
    zmq::message_t quit_message(quit.c_str(), quit.size());
    inproc_status_socket_.send(quit_message);
    for (auto& worker : workers_) {
        worker.stop();
    }

    logger_->info() << "Server shut down.";
}

void http_server::forward_as_req(zmq::socket_t& from, zmq::socket_t& to)
{
    // 1. Extract the identity frame of the stream socket.
    identity_t id;
    id.length = from.recv(&id.identity, id.identity.size());

    // 2. Extract the full message.
    int more;
    std::string frame;
    do {
        // Forward every part of the message
        zmq::message_t part;
        from.recv(&part);

        frame.append(static_cast<char*>(part.data()), part.size());

        size_t more_size = sizeof(more);
        from.getsockopt(ZMQ_RCVMORE, &more, &more_size);
    } while (more);

    // 3. Send message if it's not empty...
    if (!frame.empty()) {
        to.send(&id.identity, id.length, ZMQ_SNDMORE);
        zmq::message_t wire_response(frame.c_str(), frame.size());
        to.send(wire_response);
    }
}

void http_server::forward_as_stream(zmq::socket_t& from, zmq::socket_t& to)
{
    // 1. Extract the identity frame of the stream socket.
    identity_t id;
    id.length = from.recv(&id.identity, id.identity.size());

    // 2. Extract the full message.
    int more;
    std::string frame;
    do {
        // Forward every part of the message
        zmq::message_t part;
        from.recv(&part);

        frame.append(static_cast<char*>(part.data()), part.size());

        size_t more_size = sizeof(more);
        from.getsockopt(ZMQ_RCVMORE, &more, &more_size);
    } while (more);

    // 3. Send message if it's not empty...
    if (!frame.empty()) {
        to.send(&id.identity, id.length, ZMQ_SNDMORE);
        zmq::message_t wire_response(frame.c_str(), frame.size());
        to.send(wire_response, ZMQ_SNDMORE);

        to.send(&id.identity, id.length, ZMQ_SNDMORE);
        to.send(nullptr, 0, ZMQ_SNDMORE);
    }
}

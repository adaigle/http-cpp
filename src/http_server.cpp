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

#include "logger.hpp"
#include "http_structure.hpp"

using namespace std::chrono_literals;

http_server::http_server(uint8_t io_threads) :
    context(io_threads), http_socket(context, zmq::socket_type::stream),
    inproc_status_socket(context, zmq::socket_type::pub), inproc_request_socket(context, zmq::socket_type::dealer)
{
    try {
        inproc_status_socket.bind("inproc://http_workers_status");
    } catch (std::exception& e) {
        logger::error() << "Server error, cannot bind the HTTP worker status channel: " << logger::endl;
        logger::error() << e.what() << logger::endl;
        throw e;
    }
    try {
        inproc_request_socket.bind("inproc://http_workers_requests");
    } catch (std::exception& e) {
        logger::error() << "Server error, cannot bind the HTTP worker request channel: " << logger::endl;
        logger::error() << e.what() << logger::endl;
        throw e;
    }
}

void http_server::connect(uint16_t port, const std::string& website_root, const std::string& website_name)
{
    logger::trace() << "Connecting to port " << port << " for website '" << website_name << "'..." << logger::endl;

    // TODO: Check if the port is already used.
    //if (find_it != http_sockets.cend())
    //    throw std::invalid_argument("The binding of the socket was unsuccessful. Did you register two websites under the same port ?");

    // Check if the website directory exists.
    if (!boost::filesystem::is_directory(website_root))
        throw std::invalid_argument("Invalid website root directory. The directory must exist on the filesystem.");

    const auto insert_iter = websites.emplace(port, website_root, website_name);
    if (!insert_iter.second) {
        throw std::invalid_argument("Invalid website identifier. Is the port and name combination already used ?");
    }

    try {
        // Executing the binding.
        std::ostringstream address_builder;
        address_builder << "tcp://*:" << port;
        http_socket.bind(address_builder.str());
    } catch (...) {
        websites.erase(insert_iter.first);
        throw;
    }
}

void http_server::run()
{
    // Launch the worker threads...
    logger::debug() << "Launching worker thread..." << logger::endl;
    for (size_t i = 0; i < 4; ++i) {
        workers.emplace_front(context, i, websites);
        workers.front().start();
    }

    logger::debug() << "Sending start signal..." << logger::endl;
    const std::string start = "START";
    zmq::message_t start_message(start.c_str(), start.size());
    inproc_status_socket.send(start_message);

    std::this_thread::sleep_for(500ms);

    logger::done() << "Server ready." << logger::endl;

    // zmq_proxy doesn't work for stream sockets (identity is not sent along with the message to the same service).
    // We do the polling loop manually...

    //  Initialize poll set
    std::vector<zmq::pollitem_t> poll_items = {
        zmq::pollitem_t{static_cast<void*>(http_socket), 0, ZMQ_POLLIN, 0},
        zmq::pollitem_t{static_cast<void*>(inproc_request_socket),  0, ZMQ_POLLIN, 0}
    };

    try {
        while (true) {
            zmq::poll(poll_items, -1);

            if (poll_items[0].revents & ZMQ_POLLIN) {
                ///////////////////////////////////////////////
                // Forward incoming HTTP request to a worker.
                forward_as_req(http_socket, inproc_request_socket);
            }

            if (poll_items[1].revents & ZMQ_POLLIN) {
                ///////////////////////////////////////////////
                // Forward the HTTP response back to the client.
                forward_as_stream(inproc_request_socket, http_socket);
            }
        }
    } catch (zmq::error_t& e) {
        logger::error() << "Server error, proxy failed due to the following zmq exception: " << logger::endl;
        logger::error() << "Error " << zmq_errno() << ": " << e.what() << logger::endl;
        throw e;
    }

    logger::debug() << "Shutting down server..." << logger::endl;

    const std::string quit = "QUIT";
    zmq::message_t quit_message(quit.c_str(), quit.size());
    inproc_status_socket.send(quit_message);
    for (auto& worker : workers) {
        worker.stop();
    }

    logger::info() << "Server shut down." << logger::endl;
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

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

using namespace std::chrono_literals;

http_server::http_server(uint8_t io_threads) :
    context(io_threads), http_socket(context, zmq::socket_type::stream),
    inproc_status_socket(context, zmq::socket_type::pub), inproc_request_socket(context, zmq::socket_type::router)
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
    // http://blog.fanout.io/2014/02/18/fun-with-zurl-the-http-websocket-client-daemon/

    // Launch the worker threads...
    logger::debug() << "Launching worker thread..." << logger::endl;
    for (size_t i = 0; i < 4; ++i) {
        workers.emplace_front(context, i, websites);
        workers.front().start();
    }

    std::this_thread::sleep_for(500ms);

    logger::debug() << "Sending start signal..." << logger::endl;
    const std::string start = "START";
    zmq::message_t start_message(start.c_str(), start.size());
    inproc_status_socket.send(start_message);

    logger::info() << "Server ready." << logger::endl;

    try {
        zmq::proxy(static_cast<void*>(http_socket), static_cast<void*>(inproc_request_socket), nullptr);
    } catch (std::exception& e) {
        logger::error() << "Server error, proxy failed due to the following exception: " << logger::endl;
        logger::error() << e.what() << logger::endl;
        throw e;
    }

    logger::info() << "Shutting down server..." << logger::endl;

    const std::string quit = "QUIT";
    zmq::message_t quit_message(quit.c_str(), quit.size());
    inproc_status_socket.send(quit_message);
    for (auto& worker : workers) {
        worker.stop();
    }
}

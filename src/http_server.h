#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <array>
#include <cstdint>
#include <set>
#include <string>
#include <forward_list>

#include <zmq.hpp>

#include "http_website.h"
#include "http_worker.h"

class http_server
{
public:
    http_server(uint8_t io_threads = 1);

    http_server(const http_server&) = delete;
    http_server& operator=(const http_server&) = delete;

    /// \brief Connect the webserver to an endpoint corresponding to a website.
    ///
    /// \param website_path Path to the website's folder / web service's dynamic library.
    /// \param host_name The host name of the website. Note that this field is mandatory and used to filter the website.
    /// \param port The port to listen to. By default use port 80.
    /// \param website_name Friendly name for the website. Only used internally.
    void connect(const std::string& website_path, const std::string& host_name, const uint16_t port = 80, const std::string& website_name = "");
    void run();

private:
    static void forward_as_req(zmq::socket_t& from, zmq::socket_t& to);
    static void forward_as_stream(zmq::socket_t& from, zmq::socket_t& to);

    struct socket_info;

    zmq::context_t context_;
    zmq::socket_t http_socket_;
    zmq::socket_t inproc_status_socket_;
    zmq::socket_t inproc_request_socket_;

    std::set<http_website> websites_;
    std::forward_list<http_worker> workers_;
};


struct http_server::socket_info
{
    uint16_t port;
    std::unique_ptr<zmq::socket_t> socket;

    socket_info(uint16_t port, std::unique_ptr<zmq::socket_t> socket) : port(port), socket(std::move(socket)) {}
    socket_info(socket_info&& info) noexcept : port(info.port), socket(std::move(info.socket)) {}
    void operator=(socket_info&& info) noexcept {
        port = info.port;
        socket = std::move(info.socket);
    }

    socket_info(const socket_info&) = delete;
    void operator=(const socket_info&) = delete;
};


#endif

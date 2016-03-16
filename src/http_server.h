#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <array>
#include <cstdint>
#include <set>
#include <string>
#include <forward_list>

#include <zmq.hpp>

#include "http_worker.h"

class http_server
{
public:
    http_server(uint8_t io_threads = 1);

    http_server(const http_server&) = delete;
    http_server& operator=(const http_server&) = delete;

    void connect(uint16_t port, const std::string& website_root, const std::string& website_name = "");
    void run();

private:
    static void forward_as_req(zmq::socket_t& from, zmq::socket_t& to);
    static void forward_as_stream(zmq::socket_t& from, zmq::socket_t& to);

    struct socket_info;

    zmq::context_t context;
    zmq::socket_t http_socket;
    zmq::socket_t inproc_status_socket;
    zmq::socket_t inproc_request_socket;

    std::set<virtual_website> websites;
    std::forward_list<http_worker> workers;
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

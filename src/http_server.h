#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <array>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include <zmq.hpp>

#include "http_structure.hpp"
#include "http_worker.h"
#include "virtual_website.h"

class http_server
{
    static constexpr size_t IDENTITY_CAPACITY = 5;
    using identity_t = zmq_identity<IDENTITY_CAPACITY>;
    using http_transaction_t = http_transaction<IDENTITY_CAPACITY>;

public:
    http_server(uint8_t io_threads = 1);

    http_server(const http_server&) = delete;
    http_server& operator=(const http_server&) = delete;

    void connect(uint16_t port, const std::string& website_root, const std::string& website_name = "");
    void run();

private:
    bool extract_request(zmq::socket_t&, http_request&);
    bool send_response(zmq::socket_t&, http_transaction_t&);

    static std::string http_date();
    static std::string extract_host(http_request&);
    const virtual_website& find_website(uint16_t port, http_request&) const;

    struct socket_info;

    zmq::context_t context;
    std::vector<socket_info> http_sockets;
    zmq::socket_t http_socket;
    zmq::socket_t inproc_socket;

    std::set<virtual_website> websites;
    std::vector<http_worker> workers;
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

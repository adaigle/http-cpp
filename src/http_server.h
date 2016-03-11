#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <array>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include <zmq.hpp>

#include "virtual_website.h"
#include "http_structure.hpp"

class http_server
{
    static constexpr size_t IDENTITY_CAPACITY = 16;
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

    static std::string extract_host(http_request&);
    const virtual_website& find_website(uint16_t port, http_request&) const;

    std::set<virtual_website> websites;

    struct socket_info
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

    zmq::context_t context;
    std::vector<socket_info> http_sockets;
};


#endif

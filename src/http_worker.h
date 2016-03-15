#ifndef HTTP_WORKER_H
#define HTTP_WORKER_H

#include <set>
#include <thread>

#include <zmq.hpp>

#include "runnable.h"

#include "http_structure.hpp"
#include "virtual_website.h"

class http_worker : public class_thread
{
    static constexpr size_t IDENTITY_CAPACITY = 5;
    using identity_t = zmq_identity<IDENTITY_CAPACITY>;
    using http_transaction_t = http_transaction<IDENTITY_CAPACITY>;

public:
    http_worker(zmq::context_t&, size_t, const std::set<virtual_website>&);

protected:
    void run();

private:
    void handle_status(zmq::socket_t&);
    void handle_request(zmq::socket_t&);

    bool extract_request(zmq::socket_t&, http_request&);
    bool send_response(zmq::socket_t&, http_transaction_t&);

    static std::string http_date();
    static std::string extract_host(http_request&);
    const virtual_website& find_website(uint16_t port, http_request&) const;

    zmq::context_t& main_context;

    const std::atomic<size_t> worker_id;
    const std::set<virtual_website>& websites;
};

#endif

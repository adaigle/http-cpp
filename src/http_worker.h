#ifndef HTTP_WORKER_H
#define HTTP_WORKER_H

#include <set>
#include <thread>

#include <zmq.hpp>

#include "http_website.h"
#include "runnable.h"

class http_worker : public class_thread
{
public:
    http_worker(zmq::context_t&, size_t, const std::set<http_website>&);

protected:
    void run();

private:
    void handle_status(zmq::socket_t&);
    void handle_request(zmq::socket_t&);

    const http_website& find_website(const http_request&) const;

    zmq::context_t& main_context_;

    const std::atomic<size_t> identifier_;
    const std::set<http_website>& websites_;
};

#endif

#ifndef HTTP_WORKER_H
#define HTTP_WORKER_H

#include <thread>

#include <zmq.hpp>

class http_worker
{
public:
    http_worker(zmq::context_t&);
    void start();

private:

    void run();

    std::thread thread_handle;

    zmq::context_t& main_context;
    zmq::socket_t inproc_socket;
};

#endif

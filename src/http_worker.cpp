#include "http_worker.h"

http_worker::http_worker(zmq::context_t& context) :
    main_context(context), inproc_socket(main_context, zmq::socket_type::stream)
{

}

void http_worker::start()
{
    thread_handle = std::thread(&http_worker::run, this);
}

void http_worker::run()
{
}

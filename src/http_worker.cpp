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
    while (true) {
        ///////////////////////////////////////////////////
        // 1. Extract the request from the inproc messaging queue.

        ///////////////////////////////////////////////////
        // 2. Detect the website based on the host/port of the requets-URI.

        ///////////////////////////////////////////////////
        // 3. Load the appropriate ressource from the website.

        ///////////////////////////////////////////////////
        // 4. Construct the response and complete the request.
    }
}

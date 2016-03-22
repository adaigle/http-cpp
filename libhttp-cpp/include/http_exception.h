#ifndef HTTP_EXCEPTION_H
#define HTTP_EXCEPTION_H

#include <exception>
#include <stdexcept>

class http_invalid_request : public std::invalid_argument {
public:
    explicit http_invalid_request(const std::string& what_arg) :
        std::invalid_argument(what_arg) {}

    explicit http_invalid_request(const char* what_arg) :
        std::invalid_argument(what_arg) {}

    virtual ~http_invalid_request() {}
};

#endif

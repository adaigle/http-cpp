#ifndef HTTP_WEBSITE_H
#define HTTP_WEBSITE_H

#include <string>
#include <functional>

#include "http_service.h"

class http_website
{
    friend class std::hash<http_website>;
public:
    http_website(uint16_t port, const std::string& root, const std::string& name = "") noexcept;
    ~http_website();

    http_response execute(const http_request& request) const;

    bool operator==(const http_website& other) const;
    bool operator<(const http_website& other) const;

    const uint16_t port_;
    const std::string name_;
private:
    http_service service_;
};

#endif

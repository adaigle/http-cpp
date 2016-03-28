#ifndef HTTP_WEBSITE_H
#define HTTP_WEBSITE_H

#include <string>
#include <functional>

#include "http_service.h"

class http_website
{
    friend class std::hash<http_website>;
public:
    struct host {
        std::string name;
        uint16_t port;

        explicit host(const std::string& n, const uint16_t p);

        bool match(const std::string& host) const;
        bool operator==(const host& other) const;
        bool operator<(const host& other) const;
    };

    http_website(const std::string& website_path, host&& h, const std::string& website_name = "") noexcept;
    ~http_website();

    http_response execute(const http_request& request) const;

    bool operator==(const std::string& host) const;
    bool operator==(const http_website& other) const;
    bool operator<(const http_website& other) const;

    const host host_;
private:
    http_service service_;
};

#endif

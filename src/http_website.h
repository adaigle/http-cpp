#ifndef HTTP_WEBSITE_H
#define HTTP_WEBSITE_H

#include <string>
#include <functional>

#include "http_service.h"

class http_website
{
    friend class std::hash<http_website>;
public:

    http_website(const std::string& website_path, http_service::host&& h, const std::string& website_name = "") noexcept;
    ~http_website();

    http_response execute(const http_request& request) const;

    bool operator==(const std::string& host) const;
    bool operator==(const http_service::host& other) const;
    bool operator==(const http_website& other) const;
    bool operator<(const http_service::host& other) const;
    bool operator<(const http_website& other) const;

    const http_service::host& host() const;

private:
    http_service service_;
};

#endif

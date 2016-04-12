#include "http_website.h"

#include <cassert>
#include <fstream>
#include <iterator>
#include <sstream>

#include "logger.hpp"


http_website::http_website(const std::string& website_path, http_service::host&& h, const std::string& website_name /* = "" */) noexcept :
    service_(website_path, std::move(h), website_name)
{
}

http_website::~http_website()
{
}

http_response http_website::execute(const http_request& request) const
{
    return service_.execute(request);
}

bool http_website::operator==(const std::string& host) const
{
    return service_.host_.match(host);
}

bool http_website::operator==(const http_service::host& other_host) const
{
    return service_.host_ == other_host;
}

bool http_website::operator==(const http_website& other) const
{
    return service_.host_ == other.service_.host_;
}

bool http_website::operator<(const http_service::host& other_host) const
{
    return service_.host_ < other_host;
}

bool http_website::operator<(const http_website& other) const
{
    return service_.host_ < other.service_.host_;
}

const http_service::host& http_website::host() const
{
    return service_.host_;
}

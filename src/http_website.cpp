#include "http_website.h"

#include <cassert>
#include <fstream>
#include <iterator>
#include <sstream>

#include "logger.hpp"


http_website::http_website(const std::string& website_path, host&& h, const std::string& website_name /* = "" */) noexcept :
    host_(std::move(h)), service_(website_path, host_.name, website_name)
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
    return host_.match(host);
}

bool http_website::operator==(const http_website& other) const
{
    return host_ == other.host_;
}

bool http_website::operator<(const http_website& other) const
{
    return host_ < other.host_;
}

http_website::host::host(const std::string& n, const uint16_t p) :
    name(n), port(p)
{

}

bool http_website::host::match(const std::string& host) const
{
    bool is_match = false;

    is_match |= (host == name);

    const std::string fullhost = name + ":" + std::to_string(port);
    is_match |= (host == fullhost);

    return is_match;
}

bool http_website::host::operator==(const host& other) const
{
    return port == other.port && name == other.name;
}

bool http_website::host::operator<(const host& other) const
{
    return port < other.port || (port == other.port && name < other.name);
}

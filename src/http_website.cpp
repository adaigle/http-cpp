#include "http_website.h"

#include <cassert>
#include <fstream>
#include <iterator>
#include <sstream>

#include "logger.hpp"

http_website::http_website(uint16_t port, const std::string& root, const std::string& name) noexcept :
    port_(port), name_(name), service_(name, name, root)
{
}

http_website::~http_website()
{
}

http_response http_website::execute(const http_request& request) const
{
    return service_.execute(request);
}

bool http_website::operator==(const http_website& other) const
{
    return port_ == other.port_ && name_ == other.name_;
}

bool http_website::operator<(const http_website& other) const
{
    return port_ < other.port_ || (port_ == other.port_ && name_ < other.name_);
}

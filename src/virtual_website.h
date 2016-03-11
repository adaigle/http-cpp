#ifndef VIRTUAL_WEBSITE_H
#define VIRTUAL_WEBSITE_H

#include <string>
#include <functional>

#if defined(HAVE_LIBMAGIC)
#  if !defined(LIBMAGIC_MAGIC_FILE)
#    error "The path to libmagic folder must be provided."
#  endif
#  include "magic.h"
#endif

#include "http_structure.hpp"

class virtual_website
{
    friend class std::hash<virtual_website>;
public:
    virtual_website(uint16_t port, const std::string& root, const std::string& name = "") noexcept;
    ~virtual_website();

    bool lookup_ressource(const std::string& request_uri, const std::string& host, http_response&) const;

    bool operator==(const virtual_website& other) const;
    bool operator<(const virtual_website& other) const;

    const uint16_t port;
    const std::string name;
private:
    std::string detect_content_type(const std::string& resource_path) const;

    std::string root;

#if defined(HAVE_LIBMAGIC)
    static magic_t magic_handle;
#endif
};

#endif

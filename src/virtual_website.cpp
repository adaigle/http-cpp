#include "virtual_website.h"

#include <cassert>
#include <fstream>
#include <iterator>
#include <sstream>

#if defined(HAVE_LIBMAGIC)
#  include <boost/filesystem.hpp>
#endif

#include "logger.hpp"


#if defined(HAVE_LIBMAGIC)
    magic_t virtual_website::magic_handle = nullptr;
#endif

virtual_website::virtual_website(uint16_t port, const std::string& root, const std::string& name) noexcept :
    port(port), root(root), name(name)
{
#if defined(HAVE_LIBMAGIC)
    if (magic_handle == nullptr) {
        magic_handle = ::magic_open(MAGIC_ERROR | MAGIC_MIME);

        if (!boost::filesystem::exists(LIBMAGIC_MAGIC_FILE))
            logger::error() << "Database for libmagic could not be found at '" << LIBMAGIC_MAGIC_FILE << "'." << logger::endl;

        logger::trace() << "Loading magic database..." << logger::endl;
        ::magic_load(magic_handle, LIBMAGIC_MAGIC_FILE);

        const char* error = ::magic_error(magic_handle);
        if (error != nullptr) logger::warn() << "libmagic error: " << error << logger::endl;
        else logger::debug() << "Successfully loaded magic database." << logger::endl;
    }
#endif
}

virtual_website::~virtual_website()
{
#if defined(HAVE_LIBMAGIC)
    ::magic_close(magic_handle);
#endif
}

bool virtual_website::lookup_ressource(const std::string& request_uri, const std::string& host, http_response& response) const
{
    std::ostringstream path_builder;
    path_builder << root << request_uri;
    const std::string resource_path = path_builder.str();
    logger::debug() << "Accessing resource '" << resource_path << "'..." << logger::endl;

    const std::string content_type = detect_content_type(resource_path);
    logger::debug() << "Content-Type: " << content_type << logger::endl;
    response.entity_header.emplace("Content-Type", content_type);

    std::ifstream resource(resource_path, std::ios::binary);
    if (!resource)
        return false;

    std::ostringstream string_stream;
    std::copy(std::istreambuf_iterator<char>(resource),
              std::istreambuf_iterator<char>(),
              std::ostreambuf_iterator<char>(string_stream));
    response.message_body = string_stream.str();
    assert(resource.tellg() == string_stream.tellp());
    assert(resource.tellg() == response.message_body.length());

    logger::debug() << "Content-Length: " << response.message_body.length() << logger::endl;
    response.entity_header.emplace("Content-Length", std::to_string(response.message_body.length()));

    logger::trace() << "Resource successfully loaded!" << logger::endl;
    return true;
}

bool virtual_website::operator==(const virtual_website& other) const
{
    return port == other.port && name == other.name;
}

bool virtual_website::operator<(const virtual_website& other) const
{
    return port < other.port || (port == other.port && name < other.name);
}

std::string virtual_website::detect_content_type(const std::string& resource_path) const
{
    std::string content_type = "text/plain";

#if defined(HAVE_LIBMAGIC)
    const char* raw_content_type = ::magic_file(magic_handle, resource_path.c_str());
    if (raw_content_type == nullptr) {
        logger::warn() << "Could not detect content-type, defaulting to text/plain...." << logger::endl;
    } else {
        content_type = raw_content_type;
    }
#endif

    return content_type;
}

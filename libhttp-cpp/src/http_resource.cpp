#include "http_resource.hpp"

#include <exception>
#include <ios>
#include <limits>

#include <boost/filesystem.hpp>

#if defined(HAVE_LIBMAGIC)
#  include <boost/filesystem.hpp>
#endif

#include "logger.hpp"

http_resource http_resource::get_resource(const std::string& path)
{
    if (!boost::filesystem::exists(path))
        throw std::invalid_argument("Resource not found.");
    return http_resource(path);
}

http_resource::http_resource(http_resource&& other) :
    path_(std::move(other.path_)), stream_(std::move(other.stream_))
{

}

http_resource::info http_resource::fetch_resource_info(LIBMAGIC_HANDLE_ARG magic_handle)
{
    info meta_information;

    // tellg might not return an offset in bytes...
    // We use a more reliable method which reads/ignore everything.
    stream_.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = stream_.gcount();
    stream_.clear(); // Reset the eof flag set by ignore.
    stream_.seekg(0, std::ios_base::beg);
    assert(length > 0);
    meta_information.content_length = static_cast<size_t>(length);

    meta_information.content_type = get_content_type(magic_handle);

    return meta_information;
}

http_resource::http_resource(const std::string& path) :
    path_(path), stream_(path, std::ios::binary)
{
}

std::string http_resource::get_content_type(LIBMAGIC_HANDLE_ARG magic_handle)
{
    std::string content_type = "text/plain";

#if defined(HAVE_LIBMAGIC)
    if (magic_handle != nullptr) {
        const char* raw_content_type = ::magic_file(magic_handle, path_.c_str());
        if (raw_content_type == nullptr) {
            logger::warn() << "Could not detect content-type, defaulting to text/plain...." << logger::endl;
        } else {
            content_type = raw_content_type;
        }
    }
#endif

    return content_type;
}

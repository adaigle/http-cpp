#include "http_filesystem_resource.h"

#include <exception>
#include <ios>
#include <limits>

#include <boost/filesystem.hpp>

#include "http_structure.h"

#include "logger.hpp"

http_filesystem_resource::http_filesystem_resource(const std::string& request_uri, magic_handle_t magic_handle) :
    http_resource(request_uri), magic_handle_(magic_handle), stream_(request_uri, std::ios::binary)
{

}

http_resource::header_t http_filesystem_resource::fetch_resource_header()
{
    header_t header;

    // tellg might not return an offset in bytes...
    // We use a more reliable method which reads/ignore everything.
    stream_.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = stream_.gcount();
    stream_.clear(); // Reset the eof flag set by ignore.
    stream_.seekg(0, std::ios_base::beg);
    assert(length >= 0);
    header["Content-Length"] = std::to_string(static_cast<size_t>(length));
    
    header["Content-Type"] = get_content_type();
    header["Last-Modified"] = http_constants::http_date(boost::filesystem::last_write_time(request_uri_));

    return header;
}

void http_filesystem_resource::fetch_resource_content(std::ostream& stream)
{
    // Make sure the stream is valid and at the start.
    stream_.clear();
    stream_.seekg(0, std::ios_base::beg);

    std::copy(std::istreambuf_iterator<std::ostream::char_type>(stream_),
              std::istreambuf_iterator<std::ostream::char_type>(),
              std::ostreambuf_iterator<std::ostream::char_type>(stream));
}

std::string http_filesystem_resource::get_content_type()
{
    std::string content_type = "text/plain";

#if defined(HAVE_LIBMAGIC)
    if (magic_handle_ != nullptr) {
        const char* raw_content_type = ::magic_file(magic_handle_, request_uri_.c_str());
        if (raw_content_type == nullptr) {
            logger::warn() << "Could not detect content-type, defaulting to text/plain...." << logger::endl;
        } else {
            content_type = raw_content_type;
        }
    }
#endif

    return content_type;
}
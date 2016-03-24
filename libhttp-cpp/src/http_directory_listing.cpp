#include "http_directory_listing.h"

#include <boost/filesystem.hpp>

#include "logger.hpp"

http_directory_listing::http_directory_listing(const std::string& request_uri, magic_handle_t magic_handle) :
    http_filesystem_resource(request_uri, magic_handle)
{

}

http_resource::header_t http_directory_listing::fetch_resource_header()
{
    header_t header;
    header["Content-Type"] = "";
    header["Content-Length"] = "";
    header["Last-Modified"] = "";
    return header;
}

void http_directory_listing::fetch_resource_content(std::ostream& stream)
{
    boost::filesystem::directory_iterator end_it;

    for (boost::filesystem::directory_iterator it(request_uri_); it != end_it; ++it) {
        if (!boost::filesystem::is_directory(it->path()) &&
            !boost::filesystem::is_regular_file(it->path())) {
            stream << "[?] " << it->path().string() << "\r\n";
        }
    }

    // List all directories
    for (boost::filesystem::directory_iterator it(request_uri_); it != end_it; ++it) {
        if (boost::filesystem::is_directory(it->path())) {
            stream << "[DIR] " << it->path().filename().string() << "\r\n";
        }
    }

    // List all files
    for (boost::filesystem::directory_iterator it(request_uri_); it != end_it; ++it) {
        if (boost::filesystem::is_regular_file(it->path())) {
            stream << "[FILE] " << it->path().filename().string() << "\r\n";
        }
    }
}

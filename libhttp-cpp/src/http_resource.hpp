#ifndef HTTP_RESOURCE_H
#define HTTP_RESOURCE_H

#include <fstream>
#include <string>

#if defined(HAVE_LIBMAGIC)
#  include "magic.h"
#  define LIBMAGIC_HANDLE_ARG const magic_t
#else
#  define LIBMAGIC_HANDLE_ARG const void*
#endif

class http_resource
{
public:
    struct info {
        size_t content_length;
        std::string content_type;
    };

    static http_resource get_resource(const std::string& path);
    http_resource(http_resource&&);

    http_resource(const http_resource&) = delete;
    http_resource& operator=(const http_resource&) = delete;

    /// \brief Fetch the resource and return it's full info.
    ///
    /// \returns The full binary content of the resource.
    template <typename OutputStream, typename CharT = char>
    void fetch_resource_content(OutputStream& stream);

    /// \brief Get the meta information of a resource.
    ///
    /// \returns The information of the resource.
    info fetch_resource_info(LIBMAGIC_HANDLE_ARG magic_handle);

protected:
    http_resource(const std::string& path);

    template <typename OutoutStream>
    std::string get_content(OutoutStream& stream);
    std::string get_content_type(LIBMAGIC_HANDLE_ARG magic_handle);

private:
    const std::string path_;
    std::ifstream stream_;
};

template <typename OutputStream, typename CharT>
void http_resource::fetch_resource_content(OutputStream& stream)
{
    // Make sure the stream is valid and at the start.
    stream_.clear();
    stream_.seekg(0, std::ios_base::beg);

    std::copy(std::istreambuf_iterator<CharT>(stream_),
              std::istreambuf_iterator<CharT>(),
              std::ostreambuf_iterator<CharT>(stream));
}

#endif

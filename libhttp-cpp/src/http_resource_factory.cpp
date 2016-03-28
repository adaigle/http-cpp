#include "http_resource_factory.h"

#include <exception>
#include <stdexcept>

#include "http_resource.h"
#include "http_filesystem_resource.h"
#include "http_directory_listing.h"
#include "http_external_resource.h"

#include <boost/filesystem.hpp>
#include <boost/dll/import.hpp>

#include "logger.hpp"

magic_up http_filesystem_resource_factory::up_magic_handle_;

std::unique_ptr<http_resource_factory> http_resource_factory::create_resource_factory(const std::string& service_path)
{
    if (boost::filesystem::exists(service_path)) {
        if (boost::filesystem::is_directory(service_path)) {
            // If the path is a directory, create a filesystem factory.
            return std::unique_ptr<http_resource_factory>(new http_filesystem_resource_factory(service_path));
        } else if (boost::filesystem::is_regular_file(service_path)) {
            // If the path is a file, assume that it's a dynamic library and create an external factory.
            return std::unique_ptr<http_resource_factory>(new http_external_resource_factory(service_path));
        }
    }

    throw std::invalid_argument("Invalid path to the service, expecting an existing directory or a dynamic library.");
}

http_filesystem_resource_factory::http_filesystem_resource_factory(const std::string& virtual_path) noexcept :
    virtual_path_(virtual_path)
{
#if defined(HAVE_LIBMAGIC)
    if (!up_magic_handle_) {
        up_magic_handle_ = magic_up(::magic_open(MAGIC_ERROR | MAGIC_MIME));

        if (!boost::filesystem::exists(LIBMAGIC_MAGIC_FILE)) {
            logger::error() << "Database for libmagic could not be found at '" << LIBMAGIC_MAGIC_FILE << "'." << logger::endl;
            up_magic_handle_.reset();
        } else {
            logger::trace() << "Loading magic database..." << logger::endl;
            ::magic_load(up_magic_handle_.get(), LIBMAGIC_MAGIC_FILE);

            const char* error = ::magic_error(up_magic_handle_.get());
            if (error != nullptr) logger::warn() << "libmagic error: " << error << logger::endl;
            else logger::debug() << "Successfully loaded magic database." << logger::endl;
        }
    }
#endif
}

std::unique_ptr<http_resource> http_filesystem_resource_factory::create_handle(const std::string& request_uri) const noexcept
{
    std::string path = virtual_path_ + request_uri;
    assert(path.length() > 0);

    if (boost::filesystem::exists(path)) {
        if (boost::filesystem::is_directory(path)) {
            if (path.back() != '/')
                path.append("/");

            std::string index_path = path + "index.html";
            if (boost::filesystem::exists(index_path) && !boost::filesystem::is_directory(index_path)) {
                // Fallback #1: index.html
                return std::unique_ptr<http_resource>(new http_filesystem_resource(index_path, up_magic_handle_.get()));
            } else if (true) {
                // Fallback #2: directory listing
                return std::unique_ptr<http_resource>(new http_directory_listing(path, up_magic_handle_.get()));
            } else {
                return std::unique_ptr<http_resource>(nullptr);
            }
        }
    }
    return std::unique_ptr<http_resource>(new http_filesystem_resource(path, up_magic_handle_.get()));
}

http_external_resource_factory::http_external_resource_factory(const std::string& library_path) :
    library_path_(library_path)
{
    // The creator variable needs to stay in scope as long as we have a handle from the external library.
    creator_ = boost::dll::import_alias<handle_creator_fn_t>(
        library_path,
        "create_handle",
        boost::dll::load_mode::append_decorations
    );
    logger::info() << "Done!" << logger::endl;
}

std::unique_ptr<http_resource> http_external_resource_factory::create_handle(const std::string& request_uri) const noexcept
{
    return std::unique_ptr<http_resource>(creator_(request_uri));
}

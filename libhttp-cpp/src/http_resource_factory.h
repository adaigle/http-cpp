#ifndef HTTP_FILESYSTEM_RESOURCE_FACTORY_H
#define HTTP_FILESYSTEM_RESOURCE_FACTORY_H

#include <memory>
#include <string>

#include "interface/http_external_service.h"
#include "interface/generic_structure.h"

#include <boost/function.hpp>

#if defined(HAVE_LIBMAGIC)
#  if !defined(LIBMAGIC_MAGIC_FILE)
#    error "The path to libmagic folder must be provided."
#  endif
#  include "magic.h"

struct magic_deleter {
    constexpr magic_deleter() = default;
    void operator()(magic_set* ptr) const {
        ::magic_close(static_cast<magic_t>(ptr));
    }
};
    using magic_up = std::unique_ptr<magic_set, magic_deleter>;
#else
    using magic_up = std::unique_ptr<void>;
#endif

// Forward declaration of a general resource.
class http_resource;

class http_resource_factory
{
public:
    virtual ~http_resource_factory() = default;

    /// \brief Fetch the resource content in a stream format.
    ///
    /// \param stream The stream to output the content to.
    static std::unique_ptr<http_resource_factory> create_resource_factory(const std::string& service_path);

    /// \brief Fetch the resource content in a stream format.
    ///
    /// \param stream The stream to output the content to.
    virtual std::unique_ptr<http_resource> create_handle(const generic_request& request) const noexcept = 0;
};

class http_filesystem_resource_factory : public http_resource_factory
{
public:
    http_filesystem_resource_factory(const std::string& virtual_path) noexcept;
    virtual ~http_filesystem_resource_factory() = default;

    /// \brief Fetch the resource content in a stream format.
    ///
    /// \param stream The stream to output the content to.
    virtual std::unique_ptr<http_resource> create_handle(const generic_request& request) const noexcept override;
protected:
    const std::string virtual_path_;
    static magic_up up_magic_handle_;
};

class http_external_resource_factory : public http_resource_factory
{
public:
    http_external_resource_factory(const std::string& library_path);
    virtual ~http_external_resource_factory() = default;

    /// \brief Fetch the resource content in a stream format.
    ///
    /// \param stream The stream to output the content to.
    virtual std::unique_ptr<http_resource> create_handle(const generic_request& request) const noexcept override;

protected:
    using handle_creator_fn_t = http_external_service*();

    std::unique_ptr<http_external_service> service_;
    boost::function<handle_creator_fn_t> service_creator_;
    const std::string library_path_;
};

#endif

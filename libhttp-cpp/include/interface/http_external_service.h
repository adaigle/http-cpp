#ifndef HTTP_EXTERNAL_SERVICE_H
#define HTTP_EXTERNAL_SERVICE_H

#include "generic_structure.h"
#include "http_resource.h"

/// \brief Base class implementing the initialization point
///
/// \note subclasses should implemented a static construction function:
///     static http_external_service* custom_service::create_service() {
///         return new custom_service();
///     }
/// and make them available through DLL imports with Boost.Dll:
///     BOOST_DLL_ALIAS(custom_service::create_service, create_service);
class http_external_service {
public:
    /// \brief Allow external services to initialize properly.
    /// \note By default, does nothing.
    virtual void setup() {};

    /// \brief Execute an http request on an external http service.
    ///
    /// \param request The http request to execute.
    virtual std::unique_ptr<http_resource> create_resource(const generic_request& request) = 0;

    /// \brief Allow external services to release resources properly.
    /// \note By default, does nothing.
    virtual void exit() {};
};

#endif

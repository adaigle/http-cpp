#ifndef REST_SERVICE_H
#define REST_SERVICE_H

#include "router.hpp"
#include "rest_resource.h"
#include "rest_request.h"

#include "interface/http_external_service.h"

using http_router = rest_resource::function_parameter_pack::apply_t<router>;

class rest_service : public http_external_service, protected http_router
{
public:
    /// \brief Allow external services to initialize properly.
    /// \note By default, does nothing.
    virtual void setup() override {};

    /// \brief Execute an http request on an external http service.
    ///
    /// \param request The http request to execute.
    std::unique_ptr<http_resource> create_resource(const generic_request& request) override final {
        rest_request::param_t params;
        auto fn = get_dispatch(request.method, request.request_uri, params);
        return std::make_unique<rest_resource>(request.request_uri, fn, params);
    }

    /// \brief Allow external services to release resources properly.
    /// \note By default, does nothing.
    virtual void exit() override {};
};

#endif

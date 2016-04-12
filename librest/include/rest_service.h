#ifndef REST_SERVICE_H
#define REST_SERVICE_H

#include "http_router.h"
#include "rest_resource.h"

#include "interface/http_external_service.h"

class rest_service : public http_external_service
{
public:
    /// \brief Allow external services to initialize properly.
    /// \note By default, does nothing.
    virtual void setup() override {};

    /// \brief Execute an http request on an external http service.
    ///
    /// \param request The http request to execute.
    std::unique_ptr<http_resource> create_resource(const generic_request& request) override final {
        rest_resource::param_t params;
        auto fn = router.get_dispatch(request.method, request.request_uri, params);
        return std::make_unique<rest_resource>(request.request_uri, fn, params);
    }

    /// \brief Allow external services to release resources properly.
    /// \note By default, does nothing.
    virtual void exit() override {};

protected:
    http_router router;
};

#endif

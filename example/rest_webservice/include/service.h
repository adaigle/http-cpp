#ifndef SERVICE_H
#define SERVICE_H

#include "rest_service.h"

#include <boost/dll/alias.hpp>

class myservice : public rest_service
{
public:
    /// \brief Allow external services to initialize properly.
    /// \note By default, does nothing.
    virtual void setup() override {
        router.add_route(http_constants::method::m_get, "/message/:id", http_router::bind(&myservice::get_message, this));
        router.add_route(http_constants::method::m_put, "/message", http_router::bind(&myservice::put_message, this));
        router.add_route(http_constants::method::m_delete, "/message/:id", http_router::bind(&myservice::delete_message, this));
    }

    /// \brief Allow external services to release resources properly.
    /// \note By default, does nothing.
    virtual void exit() override {};

    void get_message(const generic_request&, generic_response&);
    void put_message(const generic_request&, generic_response&);
    void delete_message(const generic_request&, generic_response&);

    static http_external_service* create_service() {
        return new myservice();
    }
};

BOOST_DLL_ALIAS(myservice::create_service, create_service);

#endif

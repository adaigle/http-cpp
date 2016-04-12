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

        router.add_route(http_constants::method::m_get, "/message/:id/author", http_router::bind(&myservice::get_message_author, this));
        router.add_route(http_constants::method::m_get, "/message/:id/author/friend", http_router::bind(&myservice::get_message_author_friends, this));
    }

    /// \brief Allow external services to release resources properly.
    /// \note By default, does nothing.
    virtual void exit() override {};

    void get_message(const generic_request&, generic_response&, const rest_resource::param_t&);
    void get_message_author(const generic_request&, generic_response&, const rest_resource::param_t&);
    void get_message_author_friends(const generic_request&, generic_response&, const rest_resource::param_t&);
    void put_message(const generic_request&, generic_response&, const rest_resource::param_t&);
    void delete_message(const generic_request&, generic_response&, const rest_resource::param_t&);

    static http_external_service* create_service() {
        return new myservice();
    }
};

BOOST_DLL_ALIAS(myservice::create_service, create_service);

#endif

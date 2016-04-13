#ifndef SERVICE_H
#define SERVICE_H

#include "rest_service.h"
#include "rest_request.h"

#include <boost/dll/alias.hpp>

class myservice : public rest_service
{
public:
    /// \brief Allow external services to initialize properly.
    /// \note By default, does nothing.
    virtual void setup() override {
        add_route(http_constants::method::m_get, "/message/:id", rest_service::bind(&myservice::get_message, this));
        add_route(http_constants::method::m_put, "/message", rest_service::bind(&myservice::put_message, this));
        add_route(http_constants::method::m_delete, "/message/:id", rest_service::bind(&myservice::delete_message, this));

        add_route(http_constants::method::m_get, "/message/:id/author", rest_service::bind(&myservice::get_message_author, this));
        add_route(http_constants::method::m_get, "/message/:id/author/friend", rest_service::bind(&myservice::get_message_author_friends, this));
    }

    /// \brief Allow external services to release resources properly.
    /// \note By default, does nothing.
    virtual void exit() override {};

    void get_message(const rest_request&, generic_response&);
    void get_message_author(const rest_request&, generic_response&);
    void get_message_author_friends(const rest_request&, generic_response&);
    void put_message(const rest_request&, generic_response&);
    void delete_message(const rest_request&, generic_response&);

    static http_external_service* create_service() {
        return new myservice();
    }
};

BOOST_DLL_ALIAS(myservice::create_service, create_service);

#endif

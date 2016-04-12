#ifndef REST_RESOURCE_H
#define REST_RESOURCE_H

#include "interface/http_resource.h"
#include "interface/generic_structure.h"

#include <cassert>
#include <functional>
#include <string>

#include <boost/dll/alias.hpp>

class rest_resource : public http_resource
{
    using function_signature = void(const generic_request&, generic_response&);

public:
    rest_resource(const std::string& request_uri, std::function<function_signature> fn) :
        http_resource(request_uri), fn_(fn) {}

    void execute(const generic_request& request, generic_response& response) override final {
        if (fn_) {
            fn_(request, response);
        } else {
            response.status_code = http_constants::status::http_not_found;
        }
    }

private:
    std::function<function_signature> fn_;
};

#endif

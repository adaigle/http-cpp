#ifndef REST_RESOURCE_H
#define REST_RESOURCE_H

#include "interface/http_resource.h"
#include "interface/generic_structure.h"

#include "rest_request.h"
#include "parameter_pack.h"

#include <cassert>
#include <functional>
#include <string>

class rest_resource : public http_resource
{

public:
    using function_parameter_pack = parameter_pack_type<void, const rest_request&, generic_response&>;
    using function_signature = function_parameter_pack::fn_type;

    rest_resource(const std::string& request_uri, std::function<function_signature> fn, const rest_request::param_t& params) :
        http_resource(request_uri), fn_(fn), params_(params) {}

    void execute(const generic_request& request, generic_response& response) override final {
        if (fn_) {
            fn_(rest_request{request, params_}, response);
        } else {
            response.status_code = http_constants::status::http_not_found;
        }
    }

private:
    std::function<function_signature> fn_;
    rest_request::param_t params_;
};

#endif

#ifndef REST_REQUEST_H
#define REST_REQUEST_H

#include <map>
#include <string>

#include "interface/generic_structure.h"

struct rest_request
{
    using param_t = std::map<std::string, std::string>;

    const generic_request& req;
    const param_t          params;
};

#endif

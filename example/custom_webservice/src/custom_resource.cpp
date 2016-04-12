#include "custom_resource.h"

#include <iostream>

custom_resource::custom_resource(const std::string& request_uri) :
    http_resource(request_uri)
{
    std::cout << "Constructor: " << request_uri << std::endl;
}

void custom_resource::execute(const generic_request& request, generic_response& response)
{
    response.header["Content-Type"] = "";
    response.header["Content-Length"] = "";
    response.header["Last-Modified"] = "";

    std::cout << "Content" << std::endl;
    response.message_body = "Content\r\n";
}

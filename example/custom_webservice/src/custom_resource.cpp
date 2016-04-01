#include "custom_resource.h"

#include <iostream>

custom_resource::custom_resource(const std::string& request_uri) :
    http_resource(request_uri)
{
    std::cout << "Constructor: " << request_uri << std::endl;
}

void custom_resource::execute(const http_request& request, http_response& response)
{
    response.entity_header["Content-Type"] = "";
    response.entity_header["Content-Length"] = "";
    response.entity_header["Last-Modified"] = "";

    std::cout << "Content" << std::endl;
    response.message_body = "Content\r\n";
}

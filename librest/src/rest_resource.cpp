#include "rest_resource.h"

#include <iostream>

rest_resource::rest_resource(const std::string& request_uri) :
    http_resource(request_uri)
{
    std::cout << "Constructor: " << request_uri << std::endl;
}

rest_resource::header_t rest_resource::fetch_resource_header()
{
    std::cout << "Header" << std::endl;

    header_t header;
    header["Content-Type"] = "";
    header["Content-Length"] = "";
    header["Last-Modified"] = "";
    return header;
}

void rest_resource::fetch_resource_content(std::ostream& stream)
{
    std::cout << "Content" << std::endl;
    stream << "Content\r\n";
}

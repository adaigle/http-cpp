#include "http_external_resource.h"

http_external_resource::http_external_resource(const std::string& request_uri) :
    http_resource(request_uri)
{

}

http_resource::header_t http_external_resource::fetch_resource_header()
{
    header_t header;
    return header;
}

void http_external_resource::fetch_resource_content(std::ostream& stream)
{

}

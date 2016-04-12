#include "service.h"

#include <iostream>

void myservice::get_message(const generic_request& request, generic_response& response)
{
    std::cout << "GET /message/" << std::endl;
    response.message_body = "GET message";
}

void myservice::put_message(const generic_request& request, generic_response& response)
{
    std::cout << "PUT /message/" << std::endl;
    response.message_body = "PUT message";
}

void myservice::delete_message(const generic_request& request, generic_response& response)
{
    std::cout << "DELETE /message/" << std::endl;
    response.message_body = "DELETE message";
}

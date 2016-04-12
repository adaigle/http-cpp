#include "service.h"

#include <iostream>

void myservice::get_message(const generic_request& request, generic_response& response, const rest_resource::param_t& params)
{
    std::cout << "GET /message/:id" << std::endl;
    response.message_body = "GET message";
    for (const auto& pair : params) {
        response.message_body += "\n" + pair.first + ":" + pair.second;
    }
}

void myservice::get_message_author(const generic_request& request, generic_response& response, const rest_resource::param_t& params)
{
    std::cout << "GET /message/:id/author" << std::endl;
    response.message_body = "GET author";
    for (const auto& pair : params) {
        response.message_body += "\n" + pair.first + ":" + pair.second;
    }
}

void myservice::get_message_author_friends(const generic_request& request, generic_response& response, const rest_resource::param_t& params)
{
    std::cout << "GET /message/:id/author/friend" << std::endl;
    response.message_body = "GET friends";
    for (const auto& pair : params) {
        response.message_body += "\n" + pair.first + ":" + pair.second;
    }
}

void myservice::put_message(const generic_request& request, generic_response& response, const rest_resource::param_t& params)
{
    std::cout << "PUT /message/" << std::endl;
    response.message_body = "PUT message";
    for (const auto& pair : params) {
        response.message_body += "\n" + pair.first + ":" + pair.second;
    }
}

void myservice::delete_message(const generic_request& request, generic_response& response, const rest_resource::param_t& params)
{
    std::cout << "DELETE /message/" << std::endl;
    response.message_body = "DELETE message";
    for (const auto& pair : params) {
        response.message_body += "\n" + pair.first + ":" + pair.second;
    }
}

#ifndef HTTP_ROUTER_H
#define HTTP_ROUTER_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <type_traits>

#include "http_constants.h"

#include "interface/generic_structure.h"
#include "rest_request.h"

class type_dispatch {
public:
    virtual bool match(const std::string& str) const;

protected:
    std::string name;
};

template <typename Fn>
struct url_dispatch_node {
    std::map<std::string, std::unique_ptr<url_dispatch_node>> static_routes_;
    std::string dynamic_route_identifier_;
    std::unique_ptr<url_dispatch_node> dynamic_route_node_;

    // Function at the current route level.
    std::map<http_constants::method, std::function<Fn>> functions_;

    url_dispatch_node() :
        dynamic_route_identifier_(""),
        dynamic_route_node_{nullptr} {}
};

///////////////////////////////////////////////////////////
// Documentation of dispatch route format.
//
// * Every route starts with '/'.
// * Sub-resources are separated by '/'.
// * No forwarding for any routes, full path needs to be explicit.
// * Identifiers are encoded with the following format: <variable_name:type>
// * Explicit routes are always prefered to variable-encoded path.
//
// eg. /message/<id:int>
// eg. /message
// eg. /author/<id:uint>/message/<page_start:uint>-<page_end:uint>
//
// The supported types are:
// * int    -> int
// * uint   -> size_t
// * string -> std::string
//
///////////////////////////////////////////////////////////

template <typename ReturnType, typename... Params>
class router
{
    template <typename T>
    using member_dispatch_signature = ReturnType(T::*)(Params...);

public:
    using dispatch_signature = ReturnType(Params...);

    void add_route(const http_constants::method m, const std::string& dispatch_route, std::function<dispatch_signature> fn);

    std::function<dispatch_signature> get_dispatch(const http_constants::method m, const std::string& dispatch_route, rest_request::param_t& out_params) const;

    template <typename T>
    static std::function<dispatch_signature> bind(member_dispatch_signature<T> fn, T* this_ptr);

private:
    using dispatch_node = url_dispatch_node<dispatch_signature>;

    dispatch_node router_dispatch_;
};

template <typename ReturnType, typename... Params>
template <typename T>
std::function<typename router<ReturnType, Params...>::dispatch_signature> router<ReturnType, Params...>::bind(member_dispatch_signature<T> fn, T* this_ptr)
{
    using namespace std::placeholders;
    return std::bind(fn, this_ptr, _1, _2);
}

#endif

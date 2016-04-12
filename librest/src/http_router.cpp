#include "http_router.h"

#include <exception>
#include <regex>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

void http_router::add_route(const http_constants::method m, const std::string& dispatch_route, std::function<http_router::dispatch_signature> fn)
{
    if (dispatch_route.empty() || dispatch_route.front() != '/')
        throw std::invalid_argument("The dispatch route must be non empty and start with a leading '/'.");

    // Remove the leading '/'.
    std::string rel_route = dispatch_route.substr(1);

	std::vector<std::string> dispatch_split;
	boost::split(dispatch_split, rel_route, boost::is_any_of("/"));

    dispatch_node* node = &router_dispatch_;
    std::string total_path;
    for (const std::string& route : dispatch_split) {
        total_path += "/" + route;

        // 1. Check if the current route is static or dynamic
        static std::regex dynamic_route(":([a-zA-Z_][a-zA-Z0-9_]*)", std::regex_constants::ECMAScript | std::regex_constants::optimize);

        std::smatch matches;
        if (std::regex_match(route, matches, dynamic_route)) {
            // Dynamic route
            if (node->dynamic_route_identifier_.empty()) {
                node->dynamic_route_identifier_ = matches[1];
                node->dynamic_route_node_.reset(new dispatch_node);
            } else {
                if (node->dynamic_route_identifier_ != matches[1])
                    throw std::invalid_argument("A dynamic route with a different name already exists at '" + total_path + "'.");
            }
            node = node->dynamic_route_node_.get();
        } else {
            // Static route
            auto it = node->static_routes_.find(route);
            if (it == node->static_routes_.cend()) {
                auto pair = node->static_routes_.emplace(route, std::make_unique<dispatch_node>());
                assert(pair.second);
                it = pair.first;
            }
            node = it->second.get();
        }
    }

    assert(node != nullptr);

    // Check if the method is already implemented on that endpoint...
    const auto it = node->functions_.find(m);
    if (it != node->functions_.cend())
        throw std::invalid_argument("The method is already implemented on the endpoint specified.");

    node->functions_.emplace(m, fn);
}

std::function<http_router::dispatch_signature> http_router::get_dispatch(const http_constants::method m, const std::string& dispatch_route, param_t& out_params)
{
    if (dispatch_route.empty() || dispatch_route.front() != '/')
        throw std::invalid_argument("The dispatch route must be non empty and start with a leading '/'.");

    // Remove the leading '/'.
    std::string rel_route = dispatch_route.substr(1);

	std::vector<std::string> dispatch_split;
	boost::split(dispatch_split, rel_route, boost::is_any_of("/"));

    dispatch_node* node = &router_dispatch_;
    std::string total_path;
    for (const std::string& route : dispatch_split) {
        total_path += "/" + route;

        // No route can be empty.
        // TODO: Improve handling of invalid routes.
        if (route.empty())
            break;

        // Check for static route first.
        const auto it = node->static_routes_.find(route);
        if (it != node->static_routes_.cend()) {
            node = it->second.get();
        } else {
            // If no static routes are found, fallback to dynamic route.
            if (node->dynamic_route_node_) {
                assert(!node->dynamic_route_identifier_.empty());
                assert(out_params.find(node->dynamic_route_identifier_) == out_params.cend());

                out_params.emplace(node->dynamic_route_identifier_, route);
                node = node->dynamic_route_node_.get();
            } else {
                return nullptr;
            }
        }
    }

    assert(node != nullptr);

    const auto it = node->functions_.find(m);
    if (it == node->functions_.cend()) {
        return nullptr;
    }

    return it->second;
}

bool http_router::dispatch(const http_constants::method m, const std::string& dispatch_route, const generic_request& request, generic_response& response)
{
    param_t params;
    std::function<dispatch_signature> fn = get_dispatch(m, dispatch_route, params);

    if (fn) {
        fn(request, response, params);
        return true;
    }
    return false;
}

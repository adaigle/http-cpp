#ifndef HTTP_SERVICE_HPP
#define HTTP_SERVICE_HPP

#include "http_service.h"
#include "http_structure.hpp"

#include <functional>
#include <string>
#include <type_traits>

struct execution_handler_identifier {
    std::string            http_version;
    http_constants::method method;

    bool operator==(const execution_handler_identifier& other) const {
        return http_version == other.http_version && method == other.method;
    }
};

namespace std
{

template <typename... Ts>
struct hash_combine
{
	size_t operator()(const Ts&... ts) const;
};

template <typename T, typename... Ts>
struct hash_combine<T, Ts...>
{
	size_t operator()(const T& t, const Ts&... ts) const {
		const size_t seed = hash_combine<Ts...>()(ts...);
		return seed ^ (hash<T>()(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
	}
};

template <typename T>
struct hash_combine<T> {
	size_t operator()(const T& t) const { return hash<T>()(t); }
};

template <>
struct hash<http_constants::method>
{
	size_t operator()(const http_constants::method& x) const {
		return hash<typename std::underlying_type<http_constants::method>::type>()(
            static_cast<typename std::underlying_type<http_constants::method>::type>(x));
	}
};

template <>
struct hash<execution_handler_identifier>
{
	size_t operator()(const execution_handler_identifier& x) const {
		return hash_combine<decltype(x.http_version), decltype(x.method)>()(x.http_version, x.method);
	}
};

}

#endif

#ifndef HTTP_SERVICE_HPP
#define HTTP_SERVICE_HPP

#include "http_service.h"
#include "http_structure.h"

#include <functional>
#include <string>
#include <type_traits>

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

}


template <typename Stream>
Stream& operator<<(Stream& stream, const http_service::host& h)
{
    return stream << h.name << ":" << h.port;
}

#endif

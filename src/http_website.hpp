#ifndef HTTP_WEBSITE_HPP
#define HTTP_WEBSITE_HPP

#include "http_website.h"

namespace std
{

template <typename... Ts>
struct hash_combine
{
	size_t operator()(const Ts&... ts) const;
};

template <>
struct hash<http_website>
{
	size_t operator()(const http_website& x) const {
		return hash_combine<decltype(x.port), decltype(x.name)>()(x.port, x.name);
	}
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
struct hash {
	inline size_t operator()(const T& t) const { return hash<T>()(t); }
};

}

#endif

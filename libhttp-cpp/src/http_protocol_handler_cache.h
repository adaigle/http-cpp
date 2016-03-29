#ifndef HTTP_PROTOCOL_HANDLER_CACHE_H
#define HTTP_PROTOCOL_HANDLER_CACHE_H

#include <map>
#include <memory>
#include <string>

// Forward declaration of the http protocol handler.
class http_protocol_handler;

class http_protocol_handler_cache {
public:
    using key_type = std::string;
    using value_type = std::unique_ptr<http_protocol_handler>;
    using raw_value_type = http_protocol_handler*;

    http_protocol_handler_cache() = default;
    ~http_protocol_handler_cache() = default;
    http_protocol_handler_cache(const http_protocol_handler_cache&) = delete;
    http_protocol_handler_cache& operator=(const http_protocol_handler_cache&) = delete;
    http_protocol_handler_cache(http_protocol_handler_cache&&) = default;
    http_protocol_handler_cache& operator=(http_protocol_handler_cache&&) = default;

    bool contains(const key_type& key) const noexcept;

    raw_value_type get(const key_type& key) const;
    raw_value_type insert(key_type&& key, value_type&& value);

private:
    using map_type = std::map<key_type, value_type>;

    map_type    cache_;
};

#endif

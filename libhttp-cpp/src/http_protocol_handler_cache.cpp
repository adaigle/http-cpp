///////////////////////////////////////////////////////////
// Forward declarations
#include "http_protocol_handler.h"

///////////////////////////////////////////////////////////
// Class declaration
#include "http_protocol_handler_cache.h"

bool http_protocol_handler_cache::contains(const key_type& key) const noexcept
{
    const auto it = cache_.find(key);
    return (it != cache_.cend());
}

http_protocol_handler_cache::raw_value_type http_protocol_handler_cache::get(const key_type& key) const
{
    return cache_.at(key).get();
}

http_protocol_handler_cache::raw_value_type http_protocol_handler_cache::insert(key_type&& key, value_type&& value)
{
    auto pair = cache_.emplace(std::move(key), std::move(value));
    if (pair.second)
        return pair.first->second.get();
    return nullptr;
}

#include "http_protocol_handler.h"

#include <cassert>

template <typename T>
http_protocol_handler* http_protocol_handler::make_handle_impl() noexcept
{
    const auto it = protocol_handler_cache.find(T::http_version);
    if (it != protocol_handler_cache.cend()) {
        http_protocol_handler* raw_ptr = it->second.get();
        assert(raw_ptr != nullptr);
        return raw_ptr;
    }

    try {
        const auto result = protocol_handler_cache.emplace(T::http_version, std::make_unique<T>());
        assert(result.second);
        http_protocol_handler* raw_ptr = result.first->second.get();
        assert(raw_ptr != nullptr);
        return raw_ptr;
    } catch(...) {
        assert(false);
    }
    return nullptr;
}

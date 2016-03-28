#include "http_protocol_handler.h"

#include "http_resource_factory.h"

#include <cassert>

#include "logger.hpp"

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
        const auto result = protocol_handler_cache.emplace(T::http_version, std::unique_ptr<http_protocol_handler>(new T));
        assert(result.second);
        http_protocol_handler* raw_ptr = result.first->second.get();
        assert(raw_ptr != nullptr);
        return raw_ptr;
    } catch(std::exception& e) {
        logger::error() << e.what() << logger::endl;
        assert(false);
    }
    return nullptr;
}

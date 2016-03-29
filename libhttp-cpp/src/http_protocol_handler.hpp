#include "http_protocol_handler.h"

#include "http_resource_factory.h"

#include <cassert>

#include "logger.hpp"

template <typename T>
http_protocol_handler* http_protocol_handler::make_handle_impl(http_protocol_handler_cache& cache) noexcept
{
    if (cache.contains(T::http_version)) {
        http_protocol_handler* raw_ptr = cache.get(T::http_version);
        assert(raw_ptr != nullptr);
        return raw_ptr;
    }

    try {
        http_protocol_handler* raw_ptr = cache.insert(T::http_version, std::make_unique<T>());
        assert(raw_ptr != nullptr);
        return raw_ptr;
    } catch(std::exception& e) {
        logger::error() << e.what() << logger::endl;
        assert(false);
    }
    return nullptr;
}

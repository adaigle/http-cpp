#include "http_protocol_handler.h"

#include "http_resource_factory.h"

#include <cassert>

template <typename T>
http_protocol_handler* http_protocol_handler::make_handle_impl(const std::string& service_path) noexcept
{
    const auto it = protocol_handler_cache.find(T::http_version);
    if (it != protocol_handler_cache.cend()) {
        http_protocol_handler* raw_ptr = it->second.get();
        assert(raw_ptr != nullptr);
        return raw_ptr;
    }

    try {
        std::unique_ptr<http_resource_factory> factory = http_resource_factory::create_resource_factory(service_path);
        const auto result = protocol_handler_cache.emplace(T::http_version, std::unique_ptr<http_protocol_handler>(new T(std::move(factory))));
        assert(result.second);
        http_protocol_handler* raw_ptr = result.first->second.get();
        assert(raw_ptr != nullptr);
        return raw_ptr;
    } catch(...) {
        assert(false);
    }
    return nullptr;
}

#ifndef ZMQ_UTILITY_HPP
#define ZMQ_UTILITY_HPP

#include <array>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <ostream>

template <size_t N>
struct zmq_identity
{
    std::array<uint8_t, N> identity;
    size_t                 length;
};

template <size_t N>
std::ostream& operator<< (std::ostream& stream, const zmq_identity<N>& id)
{
    const auto previous_flags = stream.flags();
    const auto previous_fill = stream.fill();
    const auto previous_width = stream.width(2);

    stream.flags(stream.hex);
    stream.fill('0');
    stream.width(2);
    for (size_t i = 0; i < id.length; ++i) {
        stream << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint32_t>(id.identity[i]);
        if (i % 2 == 1) stream << ":";
    }
    stream.flags(previous_flags);
    stream.fill(previous_fill);
    stream.width(previous_width);
    return stream;
}

#endif

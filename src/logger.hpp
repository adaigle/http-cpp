#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <string>

// TODO: Use logging library...
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

template <typename CharT,
          typename Traits = std::char_traits<CharT>>
class null_buffer : public std::basic_streambuf<CharT, Traits>
{
public:
    int overflow(int c) { return c; }
};

template <typename CharT,
          typename Traits = std::char_traits<CharT>>
class basic_logger
{
private:
    enum windows_console_color
    {
    	black = 0,
    	dark_blue = 1,
    	dark_green = 2,
    	dark_aqua, dark_cyan = 3,
    	dark_red = 4,
    	dark_purple = 5, dark_pink = 5, dark_magenta = 5,
    	dark_yellow = 6,
    	dark_white = 7,
    	gray = 8,
    	blue = 9,
    	green = 10,
    	aqua = 11, cyan = 11,
    	red = 12,
    	purple = 13, pink = 13, magenta = 13,
    	yellow = 14,
    	white = 15
    };

public:
    enum class type : uint8_t {
        trace = 0,
        debug,
        info,
        info_green,
        warning,
        error,
        off
    };

    static const std::array<std::basic_string<CharT, Traits>, 6> MESSAGE_TYPE_STRING;// = {"TRACE", "DEBUG", "INFO", "WARNING", "ERROR"};
    static constexpr std::array<windows_console_color, 6> TYPE_COLOR = {
        windows_console_color::dark_white,
        windows_console_color::purple,
        windows_console_color::cyan,
        windows_console_color::green,
        windows_console_color::yellow,
        windows_console_color::red
    };
    static constexpr std::array<windows_console_color, 6> MESSAGE_COLOR = {
        windows_console_color::dark_white,
        windows_console_color::dark_white,
        windows_console_color::cyan,
        windows_console_color::green,
        windows_console_color::yellow,
        windows_console_color::red
    };

    static bool check_log(type);
    static void log_level(type);
    static void enable_wire(bool);

    static std::basic_ostream<CharT, Traits>& log(type t);

    static std::basic_ostream<CharT, Traits>& wire();
    static std::basic_ostream<CharT, Traits>& trace();
    static std::basic_ostream<CharT, Traits>& debug();
    static std::basic_ostream<CharT, Traits>& info();
    static std::basic_ostream<CharT, Traits>& done();
    static std::basic_ostream<CharT, Traits>& warn();
    static std::basic_ostream<CharT, Traits>& error();

    static std::basic_ostream<CharT, Traits>& endl(std::basic_ostream<CharT, Traits>& os);

private:
    static std::basic_ostream<CharT, Traits>& get_stream(type t);

    static constexpr auto DEFAULT_COLOR = windows_console_color::dark_white;
    static type level;
    static bool wire_logging;
};

template <typename CharT, typename Traits>
constexpr std::array<typename basic_logger<CharT, Traits>::windows_console_color, 6> basic_logger<CharT, Traits>::TYPE_COLOR;

template <typename CharT, typename Traits>
constexpr std::array<typename basic_logger<CharT, Traits>::windows_console_color, 6> basic_logger<CharT, Traits>::MESSAGE_COLOR;


template <typename CharT, typename Traits>
typename basic_logger<CharT, Traits>::type basic_logger<CharT, Traits>::level = basic_logger<CharT, Traits>::type::trace;
template <typename CharT, typename Traits>
bool basic_logger<CharT, Traits>::wire_logging = false;

using logger = basic_logger<char>;

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<< (std::basic_ostream<CharT, Traits>& stream, const typename basic_logger<CharT, Traits>::type& t)
{
    const size_t length = basic_logger<CharT, Traits>::MESSAGE_TYPE_STRING[static_cast<uint8_t>(t)].length();

    stream << "[" << basic_logger<CharT, Traits>::MESSAGE_TYPE_STRING[static_cast<uint8_t>(t)] << "]";
    for (size_t i = length; i < 5; ++i) stream << " ";
    return stream;
}


template <typename CharT, typename Traits>
bool basic_logger<CharT, Traits>::check_log(type t)
{
    return (static_cast<uint8_t>(level) <= static_cast<uint8_t>(t));
}

template <typename CharT, typename Traits>
void basic_logger<CharT, Traits>::log_level(type t)
{
    level = t;
}

template <typename CharT, typename Traits>
void basic_logger<CharT, Traits>::enable_wire(bool b)
{
    wire_logging = b;
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& basic_logger<CharT, Traits>::get_stream(type t)
{
    static null_buffer<CharT, Traits> null_buffer;
    static std::basic_ostream<CharT, Traits> null_stream(&null_buffer);
    return check_log(t) ? std::cout : null_stream;
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& basic_logger<CharT, Traits>::log(type t)
{
    std::basic_ostream<CharT, Traits>& os = get_stream(t);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, TYPE_COLOR[static_cast<uint8_t>(t)]);
    os << t << " ";
    SetConsoleTextAttribute(hConsole, MESSAGE_COLOR[static_cast<uint8_t>(t)]);
    return os;
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& basic_logger<CharT, Traits>::wire()
{
    static null_buffer<CharT, Traits> null_buffer;
    static std::basic_ostream<CharT, Traits> null_stream(&null_buffer);
    std::basic_ostream<CharT, Traits>& os = wire_logging ? std::cout : null_stream;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, windows_console_color::dark_white);
    os << "[WIRE] ";
    SetConsoleTextAttribute(hConsole, windows_console_color::dark_white);
    return os;
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& basic_logger<CharT, Traits>::trace()
{
    return log(type::trace);
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& basic_logger<CharT, Traits>::debug()
{
    return log(type::debug);
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& basic_logger<CharT, Traits>::info()
{
    return log(type::info);
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& basic_logger<CharT, Traits>::warn()
{
    return log(type::warning);
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& basic_logger<CharT, Traits>::error()
{
    return log(type::error);
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& basic_logger<CharT, Traits>::endl(std::basic_ostream<CharT, Traits>& os)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, basic_logger<CharT, Traits>::DEFAULT_COLOR);
    return os << std::endl;
}

#endif

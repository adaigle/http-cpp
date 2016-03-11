#include "logger.hpp"

template <>
const std::array<std::basic_string<char>, 6> basic_logger<char>::MESSAGE_TYPE_STRING = {"TRACE", "DEBUG", "INFO", "INFO", "WARN", "ERROR"};

template <>
const std::array<std::basic_string<wchar_t>, 6> basic_logger<wchar_t>::MESSAGE_TYPE_STRING = {L"TRACE", L"DEBUG", L"INFO", L"INFO", L"WARN", L"ERROR"}; 

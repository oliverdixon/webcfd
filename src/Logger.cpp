//
// Created by owd on 05/06/2026.
//

#include "Logger.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/console.h>
#else
#include <iostream>
#endif

namespace WebCFD
{

void Logger::log(
        const Level level,
        const std::string_view message,
        const std::source_location location
)
{
    const auto line =
            std::format("[{}] {}:{} says: {}", level_to_string(level), location.file_name(), location.line(), message);

#ifdef __EMSCRIPTEN__
    switch (level) {
    case Level::Error:
        emscripten_console_error(line.c_str());
        break;
    case Level::Info:
        emscripten_console_log(line.c_str());
        break;
    case Level::Warning:
        emscripten_console_warn(line.c_str());
        break;
    }
#else
    auto& stream = level == Level::Error ? std::cerr : std::cout;
    stream << line << '\n';
#endif
}

std::string_view Logger::level_to_string(
        const Level level
)
{
    switch (level) {
    case Level::Error:
        return "Error";
    case Level::Info:
        return "Info";
    case Level::Warning:
        return "Warning";
    }

    return "Unknown";
}

} // namespace WebCFD
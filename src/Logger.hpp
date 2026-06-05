//
// Created by owd on 05/06/2026.
//

#ifndef WEBCFD_LOGGER_H
#define WEBCFD_LOGGER_H

#include <format>
#include <source_location>
#include <string_view>

namespace WebCFD
{

class Logger
{
public:
    enum class Level
    {
        Info,
        Warning,
        Error
    };

    static void log(
            Level level,
            std::string_view message,
            std::source_location location = std::source_location::current()
    );

    template <typename... Args>
    static void log_f(
            const Level level,
            const std::source_location location,
            std::format_string<Args...> fmt,
            Args&&... args
    )
    {
        log(level, std::format(fmt, std::forward<Args>(args)...), location);
    }

private:
    static std::string_view level_to_string(Level level);
};

} // namespace WebCFD

#endif // WEBCFD_LOGGER_H

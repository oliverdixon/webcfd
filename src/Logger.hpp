/**
 * @file
 * @brief WebCFD portable logger specification
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#ifndef WEBCFD_LOGGER_H
#define WEBCFD_LOGGER_H

#include <format>
#include <source_location>
#include <string_view>

namespace WebCFD
{

/**
 * A portable (native and browser) logger implementation for formatted messages.
 *
 * If compiled for a native backend, the standard IO stream is used. If compiled for WebAssembly, the JavaScript API is
 * used to produce log messages in the browser console.
 */
class Logger
{
public:
    /**
     * Indicates the level of importance of a logged message.
     */
    enum class Level
    {
        Info,
        Warning,
        Error
    };

    /**
     * Log an unformatted message to the backend.
     *
     * @param level The level of the message.
     * @param message The unformatted message (string literal).
     * @param location The optional origin of the message.
     */
    static void log(
            Level level,
            std::string_view message,
            std::source_location location = std::source_location::current()
    );

    /**
     * Log a formatted message to the backend.
     *
     * @tparam Args Types of variadic arguments for the formatter.
     * @param level The level of the message.
     * @param location The origin of the message.
     * @param fmt The <code>printf</code>-style format string of the message.
     * @param args The values for the format string.
     */
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
    /**
     * Get a human-readable string corresponding to the given log level.
     *
     * @param level Relevant log level.
     * @return Human-readable string for the log level.
     */
    static std::string_view level_to_string(Level level);
};

} // namespace WebCFD

#endif // WEBCFD_LOGGER_H

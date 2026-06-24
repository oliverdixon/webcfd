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
        Debug,
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
     * Log a formatted message to the backend with a specific source location.
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

/**
 * @def LOG_F_DEBUG
 *
 * Conditionally logs a formatted debug-level message using WebCFD::Logger::log_f.
 *
 * @param msg The format string, followed by formatted values.
 */

#ifdef NDEBUG
#define LOG_F_DEBUG(msg, ...)
#else
#define LOG_F_DEBUG(msg, ...)                                                                                          \
    do {                                                                                                               \
        Logger::log_f(Logger::Level::Info, std::source_location::current(), msg, __VA_ARGS__);                         \
    } while (0);
#endif

/**
 * Logs a formatted info-level message using WebCFD::Logger::log_f.
 *
 * @param msg The format string, followed by formatted values.
 */
#define LOG_F_INFO(msg, ...)                                                                                           \
    do {                                                                                                               \
        Logger::log_f(Logger::Level::Info, std::source_location::current(), msg, __VA_ARGS__);                         \
    } while (0);

/**
 * Logs a formatted warning-level message using WebCFD::Logger::log_f.
 *
 * @param msg The format string, followed by formatted values.
 */
#define LOG_F_WARN(msg, ...)                                                                                           \
    do {                                                                                                               \
        Logger::log_f(Logger::Level::Warning, std::source_location::current(), msg, __VA_ARGS__);                      \
    } while (0);

/**
 * Logs a formatted error-level message using WebCFD::Logger::log_f.
 *
 * @param msg The format string, followed by formatted values.
 */
#define LOG_F_ERROR(msg, ...)                                                                                          \
    do {                                                                                                               \
        Logger::log_f(Logger::Level::Error, std::source_location::current(), msg, __VA_ARGS__);                        \
    } while (0);

/**
 * @def LOG_DEBUG
 *
 * Conditionally logs an unformatted info-level message using WebCFD::Logger::log.
 *
 * @param msg The string literal to log.
 */

#ifdef NDEBUG
#define LOG_DEBUG(msg, ...)
#else
#define LOG_DEBUG(msg, ...)                                                                                            \
    do {                                                                                                               \
        Logger::log(Logger::Level::Info, msg, std::source_location::current());                                        \
    } while (0);
#endif

/**
 * Logs an unformatted info-level message using WebCFD::Logger::log.
 *
 * @param msg The string literal to log.
 */
#define LOG_INFO(msg)                                                                                                  \
    do {                                                                                                               \
        Logger::log(Logger::Level::Info, msg, std::source_location::current());                                        \
    } while (0);

/**
 * Logs an unformatted warning-level message using WebCFD::Logger::log.
 *
 * @param msg The string literal to log.
 */
#define LOG_WARN(msg)                                                                                                  \
    do {                                                                                                               \
        Logger::log(Logger::Level::Warning, msg, std::source_location::current());                                     \
    } while (0);

/**
 * Logs an unformatted error-level message using WebCFD::Logger::log.
 *
 * @param msg The string literal to log.
 */
#define LOG_ERROR(msg)                                                                                                 \
    do {                                                                                                               \
        Logger::log(Logger::Level::Error, msg, std::source_location::current());                                       \
    } while (0);

#endif // WEBCFD_LOGGER_H

/**
 * @file
 * @brief WebCFD ConfigurationError exception specification
 * @author Oliver Dixon
 * @date 2026-06-19
 */

#ifndef WEBCFD_CONFIGURATIONERROR_H
#define WEBCFD_CONFIGURATIONERROR_H

#include <source_location>
#include <stdexcept>
#include <string_view>

namespace WebCFD
{

/**
 * A ConfigurationError indicates an error encountered during the initial configuration of the WebCFD graphics context.
 */
class ConfigurationError : public std::runtime_error
{
public:
    /**
     * Create a new ConfigurationError with the given message at the given location.
     *
     * @param message Human-readable message indicating the cause of the error.
     * @param location Optional location of the throwing statement.
     */
    explicit ConfigurationError(
            std::string_view message,
            std::source_location location = std::source_location::current()
    );

    /**
     * Retrieves the location of the throwing statement.
     *
     * @return The location of the throwing statement, if provided. Otherwise, provides the location of the
     *  ConfigurationError constructor.
     */
    [[nodiscard]] const std::source_location& where() const noexcept;

private:
    std::source_location location;
};

} // namespace WebCFD

#endif // WEBCFD_CONFIGURATIONERROR_H

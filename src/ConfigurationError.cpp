/**
 * @file
 * @brief WebCFD ConfigurationError exception implementation
 * @author Oliver Dixon
 * @date 2026-06-19
 */

#include "ConfigurationError.hpp"

#include <string>

namespace WebCFD
{

ConfigurationError::ConfigurationError(
        const std::string_view message,
        const std::source_location location
) :
    std::runtime_error(std::string(message)),
    location(location)
{
}

const std::source_location& ConfigurationError::where() const noexcept
{
    return location;
}

} // namespace WebCFD

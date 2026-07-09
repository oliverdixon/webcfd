/**
 * @file
 * @brief EchoMap ConfigurationError exception implementation
 * @author Oliver Dixon
 * @date 2026-06-19
 */

#include "ConfigurationError.hpp"

namespace echomap
{

ConfigurationError::ConfigurationError(
        const std::string_view message,
        const std::source_location location
) :
    LocatableError(message, location)
{
}

} // namespace echomap

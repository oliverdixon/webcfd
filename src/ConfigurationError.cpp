//
// Created by owd on 05/06/2026.
//

#include "ConfigurationError.hpp"

#include <string>

namespace WebCFD
{

ConfigurationError::ConfigurationError(
        const std::string_view message,
        std::source_location location
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

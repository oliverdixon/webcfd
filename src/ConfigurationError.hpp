//
// Created by owd on 05/06/2026.
//

#ifndef WEBCFD_CONFIGURATIONERROR_H
#define WEBCFD_CONFIGURATIONERROR_H

#include <source_location>
#include <stdexcept>
#include <string_view>

namespace WebCFD
{

class ConfigurationError : public std::runtime_error
{
public:
    explicit ConfigurationError(
            std::string_view message,
            std::source_location location = std::source_location::current()
    );

    [[nodiscard]] const std::source_location& where() const noexcept;

private:
    std::source_location location;
};

} // namespace WebCFD

#endif // WEBCFD_CONFIGURATIONERROR_H

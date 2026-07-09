//
// Created by owd on 05/07/2026.
//

#ifndef ECHOMAP_LOCATABLEERROR_HPP
#define ECHOMAP_LOCATABLEERROR_HPP

#include <source_location>
#include <stdexcept>
#include <string_view>

namespace echomap
{

class LocatableError : public std::runtime_error
{
public:
    /**
     * Create a new LocatableError with the given message at the given location.
     *
     * @param message Human-readable message indicating the cause of the error.
     * @param location Optional location of the throwing statement.
     */
    explicit LocatableError(
            std::string_view message,
            std::source_location location = std::source_location::current()
    );

    /**
     * Retrieves the location of the throwing statement.
     *
     * @return The location of the throwing statement, if provided. Otherwise, provides the location of the
     *  LocatableError constructor.
     */
    [[nodiscard]] const std::source_location& where() const noexcept;

private:
    std::source_location location;
};

} // namespace echomap

#endif // ECHOMAP_LOCATABLEERROR_HPP

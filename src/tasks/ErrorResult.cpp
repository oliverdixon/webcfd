//
// Created by owd on 7/9/26.
//

#include "ErrorResult.hpp"

#include <stdexcept>

namespace echomap
{

ErrorResult::ErrorResult(
        const std::string_view message
) :
    message(message)
{
}

void ErrorResult::apply(
        Project& project
)
{
    std::ignore = project;
    throw std::runtime_error(message);
}

} // namespace echomap

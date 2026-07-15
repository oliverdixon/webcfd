//
// Created by owd on 7/9/26.
//

#include "ErrorResult.hpp"

namespace echomap
{

ErrorResult::ErrorResult(
        const std::string_view message
) :
    message(message)
{
}

std::string_view ErrorResult::observe_message() const noexcept
{
    return message;
}

} // namespace echomap

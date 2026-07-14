//
// Created by owd on 7/9/26.
//

#include "ErrorResult.hpp"

#include "IResultHandler.hpp"

namespace echomap
{

ErrorResult::ErrorResult(
        const std::string_view message
) :
    IResult("ErrorResult"),
    message(message)
{
}

void ErrorResult::despatch(
        IResultHandler& handler
)
{
    handler.handle(*this);
}

std::string_view ErrorResult::observe_message() const noexcept
{
    return message;
}

} // namespace echomap

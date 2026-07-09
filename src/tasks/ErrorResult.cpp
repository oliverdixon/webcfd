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
        EchoMap& app
)
{
    std::ignore = app;
    throw std::runtime_error(message); // TODO use a proper interface on EchoMap to report errors.
}

} // namespace echomap

/**
 * @file
 *
 * ErrorResult implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-09
 */

#include "ErrorResult.hpp"

#include "../errors/LocatableError.hpp"
#include "ITask.hpp"

namespace echomap
{

ErrorResult::ErrorResult(
        const std::string_view message,
        const std::source_location location,
        std::unique_ptr<ITask> responsible_task
) :
    message(message),
    location(location),
    responsible_task(std::move(responsible_task))
{
}

ErrorResult::ErrorResult(
        const LocatableError& exception,
        std::unique_ptr<ITask> responsible_task
) :
    message(exception.what()),
    location(exception.where()),
    responsible_task(std::move(responsible_task))
{
}

std::string_view ErrorResult::what() const noexcept
{
    return message;
}

const std::source_location& ErrorResult::where() const noexcept
{
    return location;
}

const ITask* ErrorResult::observe_responsible_task() const noexcept
{
    return responsible_task.get();
}

} // namespace echomap

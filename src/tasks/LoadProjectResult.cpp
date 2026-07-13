/**
 * @file
 * @brief Project-loading result implementation
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#include "LoadProjectResult.hpp"

#include "../EchoMap.hpp"

namespace echomap
{

LoadProjectResult::LoadProjectResult(
        std::unique_ptr<Project> loaded_project
) :
    IResult(std::format("LoadProjectResult: {}", loaded_project->get_name())),
    loaded_project(std::move(loaded_project))
{
}

void LoadProjectResult::despatch(
        IResultHandler& handler
)
{
    handler.handle(*this);
}

std::unique_ptr<Project> LoadProjectResult::take_project() noexcept
{
    return std::move(loaded_project);
}

} // namespace echomap

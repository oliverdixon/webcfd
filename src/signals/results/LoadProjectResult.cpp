/**
 * @file
 * @brief Project-loading result implementation
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#include "LoadProjectResult.hpp"

#include "../../objects/Project.hpp"

namespace echomap
{

LoadProjectResult::LoadProjectResult(
        std::unique_ptr<Project> loaded_project
) :
    loaded_project(std::move(loaded_project))
{
}

LoadProjectResult::~LoadProjectResult() noexcept = default;

LoadProjectResult::LoadProjectResult(LoadProjectResult&&) noexcept = default;

LoadProjectResult& LoadProjectResult::operator=(LoadProjectResult&&) noexcept = default;

std::unique_ptr<Project> LoadProjectResult::take_project() && noexcept
{
    return std::move(loaded_project);
}

const Project* LoadProjectResult::observe_project() const noexcept
{
    return loaded_project.get();
}

} // namespace echomap

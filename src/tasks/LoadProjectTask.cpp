//
// Created by owd on 7/9/26.
//

#include "LoadProjectTask.hpp"

#include "../objects/Project.hpp"
#include "LoadProjectResult.hpp"

namespace echomap
{

LoadProjectTask::LoadProjectTask(
        const std::filesystem::path& path
) :
    project_file_path(path)
{
}

std::unique_ptr<IResult> LoadProjectTask::execute(
        const std::stop_token stop_token
)
{
    if (stop_token.stop_requested())
        return nullptr;

    return std::make_unique<LoadProjectResult>(deserialiser.deserialise_project(project_file_path));
}

} // namespace echomap

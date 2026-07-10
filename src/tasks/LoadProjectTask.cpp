/**
 * @file
 * @brief Project-loading task implementation
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#include "LoadProjectTask.hpp"

#include "../objects/Project.hpp"
#include "LoadProjectResult.hpp"

namespace echomap
{

JSONDeserialiser LoadProjectTask::deserialiser{};

LoadProjectTask::LoadProjectTask(
        const std::filesystem::path& path
) :
    project_file_path(path)
{
}

std::unique_ptr<IResult> LoadProjectTask::execute_work()
{
    return std::make_unique<LoadProjectResult>(deserialiser.deserialise_project(project_file_path));
}

} // namespace echomap

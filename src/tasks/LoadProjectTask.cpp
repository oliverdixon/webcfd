/**
 * @file
 * @brief Project-loading task implementation
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#include "LoadProjectTask.hpp"

#include "../objects/Project.hpp"
#include "LoadProjectResult.hpp"
#include "WorkerResult.hpp"

namespace echomap
{

JSONDeserialiser LoadProjectTask::deserialiser{};

LoadProjectTask::LoadProjectTask(
        const std::filesystem::path& path
) :
    ITask(std::format("LoadProjectTask: {}", path.c_str())),
    project_file_path(path)
{
}

WorkerResult LoadProjectTask::execute_work()
{
    return WorkerResult(LoadProjectResult(deserialiser.deserialise_project(project_file_path)));
}

} // namespace echomap

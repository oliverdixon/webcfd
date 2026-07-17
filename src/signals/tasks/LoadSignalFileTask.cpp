/**
 * @file
 *
 * LoadSignalFileTask implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-17
 */

#include "LoadSignalFileTask.hpp"

#include "../../objects/factories/SignalFactory.hpp"

namespace echomap
{

LoadSignalFileTask::LoadSignalFileTask(
        const std::filesystem::path& path
) :
    ITask(std::format(
            "LoadSignalTask: {}",
            path.c_str()
    )),
    path(path)
{
}

WorkerResult LoadSignalFileTask::execute_work()
{
    return LoadSignalFileResult(SignalFactory::load_wave_file(path.c_str()));
}

} // namespace echomap

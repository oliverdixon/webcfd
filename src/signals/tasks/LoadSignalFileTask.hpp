/**
 * @file
 *
 * LoadSignalFileTask specification
 *
 * @author Oliver Dixon
 * @date 2026-07-17
 */

#ifndef ECHOMAP_LOADSIGNALFILETASK_HPP
#define ECHOMAP_LOADSIGNALFILETASK_HPP

#include "ITask.hpp"

#include "../../objects/factories/SignalFactory.hpp"

namespace echomap
{

/**
 *
 */
class LoadSignalFileTask : public ITask
{
public:
    explicit LoadSignalFileTask(
            Project::id_type project_id,
            const std::filesystem::path& path,
            std::vector<std::unique_ptr<SignalFactory>>&& factories
    );

private:
    WorkerResult execute_work() override;

    Project::id_type project_id;
    std::filesystem::path path;
    std::vector<std::unique_ptr<SignalFactory>> factories;
};

} // namespace echomap

#endif // ECHOMAP_LOADSIGNALFILETASK_HPP

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

#include <filesystem>

#include "ITask.hpp"

namespace echomap
{

class SignalFactory;

/**
 *
 */
class LoadSignalFileTask : public ITask
{
public:
    explicit LoadSignalFileTask(
            id_type project_id,
            const std::filesystem::path& path,
            std::vector<std::unique_ptr<SignalFactory>>&& factories
    );

    ~LoadSignalFileTask() noexcept override;

    LoadSignalFileTask(const LoadSignalFileTask&) = delete;
    LoadSignalFileTask& operator=(const LoadSignalFileTask&) = delete;

    // TODO needs move operators.

private:
    WorkerResult execute_work() override;

    id_type project_id;
    std::filesystem::path path;
    std::vector<std::unique_ptr<SignalFactory>> factories;
};

} // namespace echomap

#endif // ECHOMAP_LOADSIGNALFILETASK_HPP

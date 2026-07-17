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

namespace echomap
{

/**
 *
 */
class LoadSignalFileTask : public ITask
{
public:
    explicit LoadSignalFileTask(const std::filesystem::path& path);

private:
    WorkerResult execute_work() override;

    std::string path;
};

} // namespace echomap

#endif // ECHOMAP_LOADSIGNALFILETASK_HPP

/**
 * @file
 *
 * ITask implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-15
 */

#include "ITask.hpp"

namespace echomap
{

WorkerResult ITask::execute(const std::stop_token& stop_token){
    if (stop_token.stop_requested())
        throw std::runtime_error("Stop requested");

    return execute_work();
}

WorkerResult ITask::operator()()
{
    return execute({});
}

} // namespace echomap

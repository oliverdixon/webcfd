/**
 * @file
 *
 * ITask implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-15
 */

#include "ITask.hpp"
#include "WorkerResult.hpp"

namespace echomap
{

WorkerResult ITask::execute(const std::stop_token& stop_token){
    if (stop_token.stop_requested())
        return WorkerResult(ErrorResult("Stop requested."));

    try {
        return execute_work();
    } catch (const std::exception& exception) {
        LOG_F_ERROR("{} failed with message: {}.", get_name(), exception.what());
        return WorkerResult(ErrorResult(exception.what()));
    } catch (...) {
        LOG_F_ERROR("{} failed with a system error.", get_name());
        return WorkerResult(ErrorResult("Unknown system error. This is a bug."));
    }
}

WorkerResult ITask::operator()()
{
    return execute({});
}

} // namespace echomap

/**
 * @file
 * @brief Worker implementation
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#include "Worker.hpp"

#include "../Logger.hpp"
#include "../objects/Project.hpp"

namespace echomap
{

Worker::Worker(
        ResultCallback result_callback
) :
    result_callback(std::move(result_callback)),
    worker_thread{[this](const std::stop_token& stop_token) {
        execute(stop_token);
    }}
{
}

void Worker::submit(
        std::unique_ptr<ITask>&& task
)
{
    LOG_F_DEBUG("Scheduling {} {}: {}.", task->get_class_name(), task->get_id(), task->get_name());
    task_queue.produce(std::move(task));
}

bool Worker::is_result_available() const noexcept
{
    return !result_queue.empty();
}

std::optional<WorkerResult> Worker::try_get_result()
{
    return result_queue.try_consume();
}

void Worker::clear()
{
    task_queue.clear();
    result_queue.clear();
}

void Worker::execute(
        const std::stop_token& stop_token
) noexcept
{
    while (!stop_token.stop_requested())
        // ThreadSafeQueue::wait_consume will block the computation thread until some work is available.
        if (auto job = task_queue.wait_consume(stop_token); job.has_value()) {
            // Likewise, ITask::execute runs the work synchronously on our computation thread.
            auto& task = *job;
            LOG_F_DEBUG("Executing {}.", task->get_name());

            try {
                auto result = task->execute(stop_token);
                LOG_F_DEBUG("Finished {}.", task->get_name());

                result_queue.produce(std::move(result));
                if (result_callback)
                    result_callback();
            } catch (const std::exception& exception) {
                result_queue.produce(ErrorResult(exception.what(), std::source_location::current(), std::move(task)));
                LOG_F_ERROR("{} failed with message: {}.", task->get_name(), exception.what());
            } catch (...) {
                result_queue.produce(ErrorResult("System error", std::source_location::current(), std::move(task)));
                LOG_F_ERROR("{} failed with a system error. This is bug.", task->get_name());
            }
        }
}

} // namespace echomap

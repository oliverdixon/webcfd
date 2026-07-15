/**
 * @file
 * @brief Worker implementation
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#include "Worker.hpp"

#include "../Logger.hpp"
#include "../objects/Project.hpp"
#include "WorkerResult.hpp"

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
    result_queue.try_consume(ephemeral_result);

    if (ephemeral_result.has_value()) {
        LOG_F_DEBUG(
                "Consuming {} {}: {}.",
                ephemeral_result->get_class_name(),
                ephemeral_result->get_id(),
                ephemeral_result->get_name()
        );

        auto result = std::move(ephemeral_result);
        ephemeral_result = std::nullopt;
        return result;
    }

    return std::nullopt;
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
            auto& task = **job;
            LOG_F_DEBUG("Executing {} {}: {}.", task.get_class_name(), task.get_id(), task.get_name());
            auto result = task.execute(stop_token);
            LOG_F_DEBUG("Finished {} {}: {}.", task.get_class_name(), task.get_id(), task.get_name());
            LOG_F_DEBUG("Publishing {} {}: {}.", result.get_class_name(), result.get_id(), result.get_name());

            result_queue.produce(std::move(result));
            if (result_callback)
                result_callback();
        }
}

} // namespace echomap

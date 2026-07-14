/**
 * @file
 * @brief Worker implementation
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#include "Worker.hpp"

#include "../Logger.hpp"
#include "ErrorResult.hpp"

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

std::unique_ptr<IResult> Worker::try_get_result()
{
    if (std::unique_ptr<IResult> result; result_queue.try_consume(result)) {
        LOG_F_DEBUG("Consuming {} {}: {}.", result->get_class_name(), result->get_id(), result->get_name());
        return std::move(result);
    }

    return nullptr;
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
            try {
                // Likewise, ITask::execute runs the work synchronously on our computation thread.
                auto& task = *job->get();
                LOG_F_DEBUG("Executing {} {}: {}.", task.get_class_name(), task.get_id(), task.get_name());
                if (auto result = task.execute(stop_token); result != nullptr) {
                    LOG_F_DEBUG("Finished {} {}: {}.", task.get_class_name(), task.get_id(), task.get_name());
                    LOG_F_DEBUG(
                            "Publishing {} {}: {}.",
                            result->get_class_name(),
                            result->get_id(),
                            result->get_name()
                    );

                    result_queue.produce(std::move(result));
                    if (result_callback)
                        result_callback();
                }

                /*
                 * Once the ITask pulled from the task queue goes out of scope here, it will be destructed. We're left
                 * with only the IResult posted on the result queue.
                 */
            } catch (const std::exception& exception) {
                result_queue.produce(std::make_unique<ErrorResult>(exception.what()));
                if (result_callback)
                    result_callback();
            }
        }
}

} // namespace echomap

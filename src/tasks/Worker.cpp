//
// Created by owd on 7/9/26.
//

#include "Worker.hpp"

#include "ErrorResult.hpp"

namespace echomap
{

Worker::Worker(WakeCallback wake_callback) :
    wake_callback(std::move(wake_callback)),
    thread{
        [this](const std::stop_token& stop_token) {
            execute(stop_token);
        }
    }
{
}

void Worker::submit(
        std::unique_ptr<ITask>&& task
)
{
    task_queue.produce(std::move(task));
}

std::unique_ptr<IResult> Worker::try_get_result()
{
    if (std::unique_ptr<IResult> result; result_queue.try_consume(result))
        return std::move(result);

    return nullptr;
}

void Worker::execute(
        const std::stop_token& stop_token
)
{
    while (!stop_token.stop_requested()) {
        auto job = task_queue.wait_consume(stop_token);
        if (!job.has_value())
            break;

        try {
            if (auto result = (*job)->execute(stop_token); result != nullptr) {
                result_queue.produce(std::move(result));
                if (wake_callback)
                    wake_callback();
            }
        } catch (const std::exception& exception) {
            result_queue.produce(std::make_unique<ErrorResult>(exception.what()));
            if (wake_callback)
                wake_callback();
        }
    }
}

} // namespace echomap

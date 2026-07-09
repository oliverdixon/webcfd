//
// Created by owd on 7/9/26.
//

#ifndef ECHOMAP_WORKER_HPP
#define ECHOMAP_WORKER_HPP

#include <functional>
#include <thread>

#include "IResult.hpp"
#include "ITask.hpp"
#include "ThreadSafeQueue.hpp"

namespace echomap
{

class Worker
{
public:
    /**
     * Callable target to invoke when a new result is produced.
     */
    using WakeCallback = std::function<void()>;

    explicit Worker(WakeCallback wake_callback = {});

    void submit(std::unique_ptr<ITask>&& task);

    std::unique_ptr<IResult> try_get_result();

private:
    void execute(const std::stop_token& stop_token);

    ThreadSafeQueue<std::unique_ptr<ITask>> task_queue;
    ThreadSafeQueue<std::unique_ptr<IResult>> result_queue;

    WakeCallback wake_callback;
    std::jthread thread;
};

} // namespace echomap

#endif // ECHOMAP_WORKER_HPP

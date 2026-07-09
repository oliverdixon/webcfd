//
// Created by owd on 7/9/26.
//

#ifndef ECHOMAP_WORKER_HPP
#define ECHOMAP_WORKER_HPP

#include <thread>

#include "IResult.hpp"
#include "ITask.hpp"
#include "ThreadSafeQueue.hpp"

namespace echomap
{

class Worker
{
public:
    Worker();

    void submit(std::unique_ptr<ITask>&& task);

    std::unique_ptr<IResult> try_get_result();

private:
    void execute(const std::stop_token& stop_token);

    ThreadSafeQueue<std::unique_ptr<ITask>> task_queue;
    ThreadSafeQueue<std::unique_ptr<IResult>> result_queue;

    std::jthread thread;
};

} // namespace echomap

#endif // ECHOMAP_WORKER_HPP

/**
 * @file
 * @brief Worker specification
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#ifndef ECHOMAP_WORKER_HPP
#define ECHOMAP_WORKER_HPP

#include <functional>
#include <thread>

#include "IResult.hpp"
#include "ITask.hpp"
#include "ThreadSafeQueue.hpp"

namespace echomap
{

/**
 * A Worker provides an encapsulated thread-safe despatch model for submitting work and reviewing results.
 *
 * <p>
 *  Clients may make use of a Worker to carry out thread-safe computation:
 *  <ol>
 *      <li>Client constructs and populates an ITask object with a description of a computation task.</li>
 *      <li>Client submits the ITask to the scheduler with @ref Worker::submit.</li>
 *      <li>Work is undertaken on a dedicated computation thread maintained by the Worker.</li>
 *      <li>Client receives the result of the computation as an IResult object from @ref Worker::try_get_result.</li>
 *  </ol>
 * </p>
 * <p>
 *  Clients may receive results of work by periodically polling the Worker (such as checking @ref Worker::try_get_result
 *  in a game loop) or through the provided asynchronous callback invoked when a new IResult is available. All Worker
 *  operations are atomic, so a single Worker instance may have clients on multiple threads.
 * </p>
 */
class Worker
{
public:
    using ResultCallback = std::function<void()>; /**< The type of callback to indicate new results. */

    /**
     * Create a new Worker with an optional callback.
     *
     * @param result_callback Client callback invoked to indicate new IResult objects available for consumption.
     */
    explicit Worker(ResultCallback result_callback = {});

    /**
     * Submit some work to the scheduler for execution on the computation thread.
     *
     * @param task A description of the task, detained within an owning container transferred to the Worker.
     */
    void submit(std::unique_ptr<ITask>&& task);

    /**
     * Checks the state of the result queue.
     *
     * @return Is there a new result available from the Worker?
     */
    [[nodiscard]] bool is_result_available() const noexcept;

    /**
     * Attempt to retrieve the latest IResult object from the computation thread.
     *
     * This function does not block.
     *
     * @return The IResult posted by the latest job, or <code>nullptr</code> if no IResult was available.
     */
    std::unique_ptr<IResult> try_get_result();

    /**
     * Clears any scheduled jobs or pending results.
     */
    void clear();

private:
    /**
     * Executor running on the computation thread to receive work from the task queue and synchronously execute.
     *
     * @param stop_token Cancellation token for the thread, provided by the thread interface.
     */
    void execute(const std::stop_token& stop_token) noexcept;

    ThreadSafeQueue<std::unique_ptr<ITask>> task_queue;
    ThreadSafeQueue<std::unique_ptr<IResult>> result_queue;

    ResultCallback result_callback; /**< Callable to inform clients of new results. */
    std::jthread worker_thread;     /**< RAII computation thread to handle ITask work pieces. */
};

} // namespace echomap

#endif // ECHOMAP_WORKER_HPP

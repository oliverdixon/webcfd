/**
 * @file
 * @brief Thread-safe queue specification and implementation
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#ifndef ECHOMAP_THREADSAFEQUEUE_HPP
#define ECHOMAP_THREADSAFEQUEUE_HPP

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

namespace echomap
{

/**
 * Provides a straightforward thread-safe double-ended queue implementation.
 *
 * Values of the templated type may be enqueued ("produced") and dequeued ("consumed") in a blocking or non-blocking
 * (but interruptable) model. All operations are atomic.
 *
 * @tparam ValueT The type of values to be queued.
 */
template <class ValueT>
    requires std::move_constructible<ValueT>
class ThreadSafeQueue
{
public:
    /**
     * Enqueue a value.
     *
     * @param value The value to enqueue.
     */
    void produce(
            ValueT value
    )
    {
        std::lock_guard lock(mutex);
        queue.push_back(std::move(value));
        cv.notify_one();
    }

    /**
     * Attempt to dequeue a value.
     *
     * This function does not block.
     *
     * @return The dequeued value, or the empty optional if there were no results available.
     */
    std::optional<ValueT> try_consume()
    {
        std::lock_guard lock(mutex);

        if (!queue.empty()) {
            auto value = std::move(queue.front());
            queue.pop_front();
            return value;
        }

        return std::nullopt;
    }

    /**
     * Attempt to dequeue a value.
     *
     * This function blocks until interrupted by the stop token or a value is available on the queue.
     *
     * @param stop_token The interrupt token for the thread block.
     * @return The dequeued value, or an empty optional if the block was interrupted prior to a value becoming
     *  available.
     */
    std::optional<ValueT> wait_consume(
            std::stop_token stop_token
    )
    {
        std::unique_lock lock(mutex);
        cv.wait(lock, stop_token, [this] {
            return !queue.empty();
        });

        if (queue.empty())
            return std::nullopt;

        auto value = std::move(queue.front());
        queue.pop_front();
        return value;
    }

    /**
     * Checks the state of the queue.
     *
     * @return Is the queue empty?
     */
    [[nodiscard]] bool empty() const noexcept
    {
        std::lock_guard lock(mutex);
        return queue.empty();
    }

    /**
     * Empties the queue.
     */
    void clear()
    {
        std::lock_guard lock(mutex);
        queue.clear();
    }

private:
    mutable std::mutex mutex;
    std::condition_variable_any cv;
    std::deque<ValueT> queue;
};

} // namespace echomap

#endif // ECHOMAP_THREADSAFEQUEUE_HPP

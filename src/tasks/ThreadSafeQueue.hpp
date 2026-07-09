//
// Created by owd on 7/9/26.
//

#ifndef ECHOMAP_THREADSAFEQUEUE_HPP
#define ECHOMAP_THREADSAFEQUEUE_HPP

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

namespace echomap
{

template <class ValueT> class ThreadSafeQueue
{
public:
    void produce(
            ValueT&& value
    )
    {
        std::lock_guard lock(mutex);
        queue.push_back(std::move(value));
        cv.notify_one();
    }

    bool try_consume(
            ValueT& out
    )
    {
        std::lock_guard lock(mutex);

        if (queue.empty())
            return false;

        out = std::move(queue.front());
        queue.pop_front();
        return true;
    }

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

private:
    std::mutex mutex;
    std::condition_variable_any cv;
    std::deque<ValueT> queue;
};

} // namespace echomap

#endif // ECHOMAP_THREADSAFEQUEUE_HPP

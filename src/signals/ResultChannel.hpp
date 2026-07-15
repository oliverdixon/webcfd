/**
 * @file
 *
 * ResultChannel specification
 *
 * @author Oliver Dixon
 * @date 2026-07-15
 */

#ifndef ECHOMAP_RESULTCHANNEL_HPP
#define ECHOMAP_RESULTCHANNEL_HPP

#include <sigc++/signal.h>

#include <exception>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace echomap
{

/**
 * Maintains a type-safe multi-observer single-consumer message bus for WorkerResult objects.
 *
 * @tparam ResultT The type of result to broadcast on the channel. In practice, this is the first parameter of the
 *  subscribed slots.
 */
template <class ResultT> class ResultChannel
{
public:
    /**
     * Provide a new slot as an observer on the channel.
     *
     * @tparam SlotT The slot type signature, accepting <code>const ResultT&</code>.
     * @param slot The slot to add to the subscribers of the channel.
     * @return The connection between the slot and the signal.
     */
    template <class SlotT>
    [[nodiscard]] sigc::connection observe(
            SlotT&& slot
    )
    {
        return observers.connect(std::forward<SlotT>(slot));
    }

    /**
     * Nominate the consumer slot for the channel.
     *
     * @tparam SlotT The slot type signature, accepting <code>ResultT&&</code>.
     * @param slot The slot to consume the result following broadcast to observers.
     *
     * @throws std::runtime_error The channel already had a nominated consumer.
     * @warning This function may only be called once for each channel; otherwise, an exception will be thrown.
     */
    template <class SlotT>
        requires(!std::is_const_v<ResultT>)
    [[nodiscard]] sigc::connection nominate_consumer(
            SlotT&& slot
    )
    {
        if (!consumer.empty())
            throw std::logic_error("Result channel already has a nominated consumer.");

        return consumer.connect(std::forward<SlotT>(slot));
    }

private:
    friend class WorkerResultDespatcher;

    using ObserverSignalT = sigc::signal<void(const ResultT&)>;
    using ConsumerSignalT = sigc::signal<void(ResultT&&)>;

    /**
     * Publish a consumable result onto the channel for all observers, followed by the single consumer.
     *
     * @param result An r-value reference to the result to publish on the channel.
     */
    void publish(
            ResultT&& result
    )
    {
        std::exception_ptr observer_exception;

        // Emit const ResultT& to all observers.
        try {
            publish(result);
        } catch (...) {
            observer_exception = std::current_exception();
        }

        // If there is a nominated consumer, send it out.
        if (!consumer.empty())
            consumer.emit(std::move(result));

        if (observer_exception)
            std::rethrow_exception(observer_exception);
    }

    /**
     * Publish a non-consumable result onto the channel for all observers.
     *
     * @param result A constant l-value reference to the result to publish on the channel.
     */
    void publish(
            const ResultT& result
    )
    {
        observers.emit(result);
    }

    ObserverSignalT observers;
    ConsumerSignalT consumer;
};

} // namespace echomap

#endif // ECHOMAP_RESULTCHANNEL_HPP

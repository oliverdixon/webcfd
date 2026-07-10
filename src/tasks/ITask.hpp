/**
 * @file
 * @brief Task description interface specification
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#ifndef ECHOMAP_ITASK_HPP
#define ECHOMAP_ITASK_HPP

#include <stop_token>

#include "IResult.hpp"

namespace echomap
{

/**
 * An ITask represents an invocable piece of work that produces an IResult.
 *
 * @tparam ReceiverT The receiver for IResult objects produced by the ITask.
 */
template<class ReceiverT>
class ITask
{
public:
    /**
     * Destruct the ITask base.
     */
    virtual ~ITask() = default;

    /**
     * Execute the work, subject to the given token, and produce an IResult.
     *
     * @param stop_token Token for cancelling execution.
     * @return An owning container detaining the result of the work, or <code>nullptr</code> if the cancelled.
     */
    std::unique_ptr<IResult<ReceiverT>> execute(
            const std::stop_token& stop_token
    )
    {
        if (stop_token.stop_requested())
            return nullptr;

        return execute_work();
    }

    /**
     * Alias for @ref execute with a vacuous stop token.
     *
     * @return An owning container detaining the result of the work.
     */
    std::unique_ptr<IResult<ReceiverT>> operator()()
    {
        return execute({});
    }

private:
    /**
     * Execute the work and produce an IResult.
     *
     * @return An owning container detaining the result of the work.
     */
    virtual std::unique_ptr<IResult<ReceiverT>> execute_work() = 0;
};

} // namespace echomap

#endif // ECHOMAP_ITASK_HPP

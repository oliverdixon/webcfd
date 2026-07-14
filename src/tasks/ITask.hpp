/**
 * @file
 * @brief Task description interface specification
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#ifndef ECHOMAP_ITASK_HPP
#define ECHOMAP_ITASK_HPP

#include <stop_token>

#include "../Logger.hpp"
#include "../objects/Object.hpp"
#include "ErrorResult.hpp"
#include "IResult.hpp"

namespace echomap
{

/**
 * An ITask represents an invocable piece of work that produces an IResult.
 */
class ITask : public Object<ITask>
{
public:
    explicit ITask(
            const std::string_view task_name
    ) :
        Object(task_name)
    {
    }

    /**
     * Destruct the ITask base.
     */
    virtual ~ITask() noexcept = default;

    /**
     * Execute the work, subject to the given token, and produce an IResult.
     *
     * @param stop_token Token for cancelling execution.
     * @return An owning container detaining the result of the work, or <code>nullptr</code> if the cancelled.
     */
    std::unique_ptr<IResult> execute(
            const std::stop_token& stop_token
    )
    {
        if (stop_token.stop_requested())
            return nullptr;

        try {
            return execute_work();
        } catch (const std::exception& exception) {
            LOG_F_ERROR("{} failed with message: {}.", get_name(), exception.what());
            return std::make_unique<ErrorResult>(exception.what());
        } catch (...) {
            LOG_F_ERROR("{} failed with a system error.", get_name());
            return std::make_unique<ErrorResult>("Unknown system error. This is a bug.");
        }
    }

    /**
     * Alias for @ref execute with a vacuous stop token.
     *
     * @return An owning container detaining the result of the work.
     */
    std::unique_ptr<IResult> operator()()
    {
        return execute({});
    }

private:
    /**
     * Execute the work and produce an IResult.
     *
     * @return An owning container detaining the result of the work.
     */
    virtual std::unique_ptr<IResult> execute_work() = 0;
};

template <> constexpr std::string Object<ITask>::class_name = "Task";

} // namespace echomap

#endif // ECHOMAP_ITASK_HPP

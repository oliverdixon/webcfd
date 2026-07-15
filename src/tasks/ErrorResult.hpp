/**
 * @file
 *
 * ErrorResult specification
 *
 * @author Oliver Dixon
 * @date 2026-07-09
 */

#ifndef ECHOMAP_ERRORRESULT_HPP
#define ECHOMAP_ERRORRESULT_HPP

#include <memory>
#include <source_location>
#include <string>
#include <string_view>

namespace echomap
{

class ITask;
class LocatableError;

/**
 * Indicates that an ITask did not successfully complete.
 */
class ErrorResult
{
public:
    /**
     * Create a new ErrorResult literally specifying the message, location, and the responsible task.
     *
     * @param message Human-readable description of the error.
     * @param location Most relevant source location of the error origin.
     * @param responsible_task The ITask (ownership transferred into the ErrorResult) responsible.
     */
    explicit ErrorResult(
            std::string_view message,
            std::source_location location,
            std::unique_ptr<ITask> responsible_task
    );

    /**
     * Create a new ErrorResult from a LocatableError and the responsible task.
     *
     * @param exception Exception thrown from the ITask.
     * @param responsible_task The ITask (ownership transferred into the ErrorResult) responsible.
     */
    explicit ErrorResult(
            const LocatableError& exception,
            std::unique_ptr<ITask> responsible_task
    );

    /**
     * Observes the human-readable error message.
     *
     * @return Description of the ErrorResult.
     */
    [[nodiscard]] std::string_view what() const noexcept;

    /**
     * Observes the location of the error origin.
     *
     * @return Location of the invoker of the ErrorResult.
     */
    [[nodiscard]] const std::source_location& where() const noexcept;

    /**
     * Retrieves an observing pointer to the ITask responsible for causing the ErrorResult.
     *
     * @return An observing pointer to the responsible ITask, or <code>nullptr</code> if the ErrorResult does not relate
     *  to a particular ITask.
     */
    [[nodiscard]] const ITask* observe_responsible_task() const noexcept;

private:
    std::string message;
    std::source_location location;
    std::unique_ptr<ITask> responsible_task;
};

} // namespace echomap

#endif // ECHOMAP_ERRORRESULT_HPP

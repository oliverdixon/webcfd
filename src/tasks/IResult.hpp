/**
 * @file
 * @brief Result description interface specification
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#ifndef ECHOMAP_IRESULT_HPP
#define ECHOMAP_IRESULT_HPP

namespace echomap
{

class IResultHandler;

/**
 * An IResult represents a callable description of results obtained following an ITask.
 */
class IResult
{
public:
    /**
     * Destruct an IResult base.
     */
    virtual ~IResult() = default;

    /**
     * Inform the given receiver of the result.
     *
     * @param handler The target to receive to results.
     */
    virtual void despatch(IResultHandler& handler) = 0;

    /**
     * Alias for @ref apply.
     *
     * @param handler The target to receive to results.
     */
    void operator()(IResultHandler& handler)
    {
        despatch(handler);
    }
};

} // namespace echomap

#endif // ECHOMAP_IRESULT_HPP

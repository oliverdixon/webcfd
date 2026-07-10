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

/**
 * An IResult represents a callable description of results obtained following an ITask.
 *
 * @tparam ReceiverT The object to receive the IResult.
 */
template <class ReceiverT> class IResult
{
public:
    /**
     * Destruct an IResult base.
     */
    virtual ~IResult() = default;

    /**
     * Inform the given receiver of the result.
     *
     * @param receiver The target to receive to results.
     */
    virtual void apply(ReceiverT& receiver) = 0;

    /**
     * Alias for @ref apply.
     *
     * @param receiver The target to receive to results.
     */
    void operator()(ReceiverT& receiver)
    {
        apply(receiver);
    }
};

} // namespace echomap

#endif // ECHOMAP_IRESULT_HPP

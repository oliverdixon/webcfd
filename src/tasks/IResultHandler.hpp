/**
 * @file
 *
 * IResultHandler specification
 *
 * @author Oliver Dixon
 * @date 2026-07-11
 */

#ifndef ECHOMAP_IRESULTHANDLER_HPP
#define ECHOMAP_IRESULTHANDLER_HPP

namespace echomap
{

class ErrorResult;
class LoadProjectResult;
class DownsampleResult;
class DFTResult;

/**
 * Defines an interface for handling IResult objects resulting from ITask computations.
 */
class IResultHandler
{
public:
    virtual ~IResultHandler() = default;

    virtual void handle(LoadProjectResult& result)
    {
        std::ignore = result;
    }

    virtual void handle(DownsampleResult& result)
    {
        std::ignore = result;
    }

    virtual void handle(DFTResult& result)
    {
        std::ignore = result;
    }

    // TODO: remove default implementation - make subscribers acknowledge errors.
    virtual void handle(const ErrorResult& result)
    {
        std::ignore = result;
    }
};

} // namespace echomap

#endif // ECHOMAP_IRESULTHANDLER_HPP

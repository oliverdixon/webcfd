/**
 * @file
 *
 * DownsampleResult implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-13
 */

#include "DownsampleResult.hpp"

#include "IResultHandler.hpp"

namespace echomap
{

DownsampleResult::DownsampleResult(
        const Signal::id_type source_id,
        std::unique_ptr<Signal> downsampled
) :
    IResult(std::format("DownsampleResult: {}", downsampled->get_name())),
    source_id(source_id),
    downsampled(std::move(downsampled))
{
}

void DownsampleResult::despatch(
        IResultHandler& handler
)
{
    handler.handle(*this);
}

std::unique_ptr<Signal> DownsampleResult::take_downsampled() noexcept
{
    return std::move(downsampled);
}

Signal::id_type DownsampleResult::get_source_id() const noexcept
{
    return source_id;
}

} // namespace echomap

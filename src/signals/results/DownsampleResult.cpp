/**
 * @file
 *
 * DownsampleResult implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-13
 */

#include "DownsampleResult.hpp"

namespace echomap
{

DownsampleResult::DownsampleResult(
        const id_type source_id,
        std::unique_ptr<Signal> downsampled
) :
    source_id(source_id),
    downsampled(std::move(downsampled))
{
    assert(this->downsampled != nullptr);
}

std::unique_ptr<Signal> DownsampleResult::take_downsampled() && noexcept
{
    return std::move(downsampled);
}

const Signal* DownsampleResult::observe_downsampled() const noexcept
{
    return downsampled.get();
}

id_type DownsampleResult::get_source_id() const noexcept
{
    return source_id;
}

} // namespace echomap

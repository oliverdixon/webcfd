/**
 * @file
 *
 * DownsampleResult specification
 *
 * @author Oliver Dixon
 * @date 2026-07-13
 */

#ifndef ECHOMAP_DOWNSAMPLERESULT_HPP
#define ECHOMAP_DOWNSAMPLERESULT_HPP

#include <memory>

#include "../../objects/Signal.hpp"

namespace echomap
{

/**
 * Denotes a completed for a downsampled Signal produced by a DownsampleTask job.
 */
class DownsampleResult
{
public:
    explicit DownsampleResult(
            id_type source_id,
            std::unique_ptr<Signal> downsampled
    );

    [[nodiscard]] std::unique_ptr<Signal> take_downsampled() && noexcept;
    [[nodiscard]] const Signal* observe_downsampled() const noexcept;
    [[nodiscard]] id_type get_source_id() const noexcept;

private:
    const id_type source_id;
    std::unique_ptr<Signal> downsampled;
};

} // namespace echomap

#endif // ECHOMAP_DOWNSAMPLERESULT_HPP

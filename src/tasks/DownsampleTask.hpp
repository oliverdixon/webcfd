/**
 * @file
 *
 * DownsampleTask specification
 *
 * @author Oliver Dixon
 * @date 2026-07-13
 */

#ifndef ECHOMAP_DOWNSAMPLETASK_HPP
#define ECHOMAP_DOWNSAMPLETASK_HPP

#include <memory>

#include "ITask.hpp"

namespace echomap
{

class Signal;

/**
 * Represents an ITask for downsampling a Signal time-series data by a fixed factor.
 *
 * @see SignalFactory::downsample
 */
class DownsampleTask : public ITask
{
    static constexpr float downsampling_factor = 50.0f;

public:
    /**
     * Creates a new downsampling job for the given Signal.
     *
     * Given the immutable nature of Signal Sample data, it is assumed that the time-series data will not be modified
     * during the scheduling and execution of the job. Shared ownership is required to preserve lifetimes across the
     * master and computation threads.
     *
     * @param signal The shared-ownership Signal to downsample.
     * @param factor The multiplicative factor by which the Signal should be downsampled.
     *
     * @pre The given Signal must detain a non-nullptr Signal.
     */
    explicit DownsampleTask(
            std::shared_ptr<Signal> signal,
            float factor = downsampling_factor
    );

private:
    std::unique_ptr<IResult> execute_work() override;

    std::shared_ptr<Signal> signal;
    const float factor;
};

} // namespace echomap

#endif // ECHOMAP_DOWNSAMPLETASK_HPP

/**
 * @file
 *
 * DFTTask specification
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#ifndef ECHOMAP_DFTTASK_HPP
#define ECHOMAP_DFTTASK_HPP

#include <memory>

#include "../../objects/factories/WindowFunctions.hpp"
#include "ITask.hpp"

namespace echomap
{

class Signal;

/**
 * Represents an ITask for computing the Discrete Fourier Transform of a Signal.
 *
 * @see FrequencySpectrumFactory::create_frequency_spectrum
 */
class DFTTask : public ITask
{
public:
    /**
     * Creates a new DFT job for the given Signal.
     *
     * Given the immutable nature of Signal Sample data, it is assumed that the time-series data will not be modified
     * during the scheduling and execution of the job. Shared ownership is required to preserve lifetimes across the
     * master and computation threads.
     *
     * @param signal The shared-ownership Signal to analyse.
     * @param window_function The window function to preprocess the input data.
     * @param transform_size The number of samples to include in the transform.
     *
     * @pre The given Signal must detain a non-nullptr Signal.
     */
    explicit DFTTask(
            std::shared_ptr<Signal> signal,
            WindowFunctions::AllFunctions window_function,
            std::size_t transform_size
    );

    ~DFTTask() noexcept override;

    DFTTask(const DFTTask&) = delete;
    DFTTask& operator=(const DFTTask&) = delete;

private:
    WorkerResult execute_work() override;

    std::shared_ptr<Signal> signal;
    const WindowFunctions::AllFunctions window_function;
    const std::size_t transform_size;
};

} // namespace echomap

#endif // ECHOMAP_DFTTASK_HPP

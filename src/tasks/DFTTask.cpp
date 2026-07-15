/**
 * @file
 *
 * DFTTask implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#include "DFTTask.hpp"

#include "../objects/Signal.hpp"
#include "../objects/factories/FrequencySpectrumFactory.hpp"
#include "DFTResult.hpp"
#include "WorkerResult.hpp"

namespace echomap
{

DFTTask::DFTTask(
        std::shared_ptr<Signal> signal,
        const FrequencySpectrum::WindowFunction window_function,
        const std::size_t transform_size
) :
    ITask(std::format(
            "DFTTask: {}",
            signal->get_name()
    )),
    signal(std::move(signal)),
    window_function(window_function),
    transform_size(transform_size)
{
    assert(this->signal != nullptr);
}

WorkerResult DFTTask::execute_work()
{
    return WorkerResult(DFTResult(
            signal->get_id(),
            FrequencySpectrumFactory::create_frequency_spectrum(*signal, window_function, transform_size),
            transform_size
    ));
}

} // namespace echomap

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

namespace echomap
{

DFTTask::DFTTask(
        std::shared_ptr<Signal> signal,
        const FrequencySpectrum::WindowFunction window_function
) :
    ITask(std::format(
            "DFTTask: {}",
            signal->get_name()
    )),
    signal(std::move(signal)),
    window_function(window_function)
{
    assert(this->signal != nullptr);
}

std::unique_ptr<IResult> DFTTask::execute_work()
{
    return std::make_unique<DFTResult>(
            signal->get_id(),
            FrequencySpectrumFactory::create_frequency_spectrum(*signal, window_function)
    );
}

} // namespace echomap

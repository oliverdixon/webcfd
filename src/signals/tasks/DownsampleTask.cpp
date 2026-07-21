/**
 * @file
 *
 * DownsampleTask implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-13
 */

#include "DownsampleTask.hpp"

#include "../../objects/FrequencySpectrum.hpp"
#include "../../objects/Project.hpp"
#include "../../objects/factories/SignalFactory.hpp"
#include "../results/DownsampleResult.hpp"

namespace echomap
{

DownsampleTask::DownsampleTask(
        std::shared_ptr<Signal> signal,
        const float factor
) :
    ITask(std::format("DownsampleTask: {}", signal->get_name())),
    signal(std::move(signal)),
    factor(factor)
{
    assert(this->signal != nullptr);
}

DownsampleTask::~DownsampleTask() noexcept = default;

WorkerResult DownsampleTask::execute_work()
{
    return DownsampleResult(signal->get_id(), SignalFactory::downsample(*signal, factor));
}

} // namespace echomap

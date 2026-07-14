/**
 * @file
 *
 * DownsampleTask implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-13
 */

#include "DownsampleTask.hpp"

#include "../objects/factories/SignalFactory.hpp"
#include "DownsampleResult.hpp"

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

std::unique_ptr<IResult> DownsampleTask::execute_work()
{
    return std::make_unique<DownsampleResult>(signal->get_id(), SignalFactory::downsample(*signal, factor));
}

} // namespace echomap

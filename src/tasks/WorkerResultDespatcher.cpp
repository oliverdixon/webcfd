/**
 * @file
 *
 * WorkerResultDespatcher implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-15
 */

#include "WorkerResultDespatcher.hpp"

#include "WorkerResult.hpp"

namespace echomap
{

void WorkerResultDespatcher::publish(
        WorkerResult&& result
)
{
    std::visit(
            [this](auto& payload) {
                publish_payload(payload);
            },
            result.result
    );
}

void WorkerResultDespatcher::publish_payload(
        const DFTResult& payload
) const
{
    dft_finished.emit(payload);
}

void WorkerResultDespatcher::publish_payload(
        const DownsampleResult& payload
) const
{
    downsample_finished.emit(payload);
}

void WorkerResultDespatcher::publish_payload(
        const LoadProjectResult& payload
) const
{
    load_project_finished.emit(payload);
}

void WorkerResultDespatcher::publish_payload(
        const ErrorResult& payload
) const
{
    error.emit(payload);
}

} // namespace echomap

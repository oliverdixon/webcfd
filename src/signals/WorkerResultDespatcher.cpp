/**
 * @file
 *
 * WorkerResultDespatcher implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-15
 */

#include "WorkerResultDespatcher.hpp"

namespace echomap
{

void WorkerResultDespatcher::publish(
        WorkerResult&& result
)
{
    std::visit(
            [this]<typename T0>(T0 payload) {
                publish_payload(std::move(payload));
            },
            std::move(result)
    );
}

void WorkerResultDespatcher::publish_payload(
        DFTResult&& payload
)
{
    dft_finished_channel.publish(std::move(payload));
}

void WorkerResultDespatcher::publish_payload(
        DownsampleResult&& payload
)
{
    downsample_finished_channel.publish(std::move(payload));
}

void WorkerResultDespatcher::publish_payload(
        LoadProjectResult&& payload
)
{
    load_project_finished_channel.publish(std::move(payload));
}

void WorkerResultDespatcher::publish_payload(
        LoadSignalFileResult&& payload
)
{
    load_signal_file_channel.publish(std::move(payload));
}

void WorkerResultDespatcher::publish_payload(
        const ErrorResult& payload
)
{
    error_channel.publish(payload);
}

} // namespace echomap

/**
 * @file
 *
 * WorkerResultDespatcher specification
 *
 * @author Oliver Dixon
 * @date 2026-07-15
 */

#ifndef ECHOMAP_WORKERRESULTDESPATCHER_HPP
#define ECHOMAP_WORKERRESULTDESPATCHER_HPP

#include "ITask.hpp"
#include "ResultChannel.hpp"

namespace echomap
{

/**
 * Manages channels for WorkerResult message routing.
 */
class WorkerResultDespatcher
{
public:
    /**
     * Publish new WorkerResult message on the suitable channel.
     *
     * @param result The message to publish.
     */
    void publish(WorkerResult&& result);

    /**
     * Channel to indicate completion of DFTTask.
     */
    ResultChannel<DFTResult> dft_finished_channel;

    /**
     * Channel to indicate completion of DownsampleTask.
     */
    ResultChannel<DownsampleResult> downsample_finished_channel;

    /**
     * Channel to indicate completion of LoadProjectTask.
     */
    ResultChannel<LoadProjectResult> load_project_finished_channel;

    /**
     * Channel to indicate emission of ErrorResult.
     */
    ResultChannel<const ErrorResult> error_channel;

private:
    void publish_payload(DFTResult&& payload);
    void publish_payload(DownsampleResult&& payload);
    void publish_payload(LoadProjectResult&& payload);
    void publish_payload(const ErrorResult& payload);
};

} // namespace echomap

#endif // ECHOMAP_WORKERRESULTDESPATCHER_HPP

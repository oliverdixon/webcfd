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

class ErrorResult;
class LoadProjectResult;
class DownsampleResult;
class DFTResult;

/**
 *
 */
class WorkerResultDespatcher
{
public:
    void publish(WorkerResult&& result);

    ResultChannel<DFTResult> dft_finished_channel;
    ResultChannel<DownsampleResult> downsample_finished_channel;
    ResultChannel<LoadProjectResult> load_project_finished_channel;
    ResultChannel<const ErrorResult> error_channel;

private:
    void publish_payload(DFTResult&& payload);
    void publish_payload(DownsampleResult&& payload);
    void publish_payload(LoadProjectResult&& payload);
    void publish_payload(const ErrorResult& payload);
};

} // namespace echomap

#endif // ECHOMAP_WORKERRESULTDESPATCHER_HPP

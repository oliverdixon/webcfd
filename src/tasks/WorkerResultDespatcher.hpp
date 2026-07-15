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

#include <sigc++/sigc++.h>

namespace echomap
{

struct WorkerResult;
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

    sigc::signal<void(const DFTResult&)> dft_finished;
    sigc::signal<void(const DownsampleResult&)> downsample_finished;
    sigc::signal<void(const LoadProjectResult&)> load_project_finished;
    sigc::signal<void(const ErrorResult&)> error;

private:
    void publish_payload(const DFTResult& payload) const;
    void publish_payload(const DownsampleResult& payload) const;
    void publish_payload(const LoadProjectResult& payload) const;
    void publish_payload(const ErrorResult& payload) const;
};

} // namespace echomap

#endif // ECHOMAP_WORKERRESULTDESPATCHER_HPP

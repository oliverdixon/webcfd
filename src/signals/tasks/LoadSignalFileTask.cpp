/**
 * @file
 *
 * LoadSignalFileTask implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-17
 */

#include "LoadSignalFileTask.hpp"

#include "../../objects/factories/SignalFactory.hpp"

namespace echomap
{

LoadSignalFileTask::LoadSignalFileTask(
        const id_type project_id,
        const std::filesystem::path& path,
        std::vector<std::unique_ptr<SignalFactory>>&& factories
) :
    ITask(std::format(
            "LoadSignalFileTask: {} ",
            path.c_str()
    )),
    project_id(project_id),
    path(path),
    factories(std::move(factories))
{
}

LoadSignalFileTask::~LoadSignalFileTask() noexcept = default;

WorkerResult LoadSignalFileTask::execute_work()
{
    // Prepare the factories as slots for the loader.
    std::vector<SignalFactory*> factory_refs;
    factory_refs.reserve(factories.size());
    for (const auto& factory_owner : factories)
        factory_refs.emplace_back(factory_owner.get());

    // Load each channel into the correct factories.
    SignalFactory::load_wave_file(path.c_str(), factory_refs);

    // Retrieve the loaded signals.
    std::vector<std::unique_ptr<Signal>> loaded_signals;
    for (auto&& factory_owner : factories)
        if (factory_owner != nullptr)
            loaded_signals.emplace_back(std::move(factory_owner)->take_signal());

    // Now we own the loaded signals, pass them onto the result.
    return LoadSignalFileResult(project_id, std::move(loaded_signals));
}

} // namespace echomap

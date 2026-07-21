/**
 * @file
 *
 * LoadSignalFileResult implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-17
 */

#include "LoadSignalFileResult.hpp"

#include "../../objects/Signal.hpp"

namespace echomap
{

LoadSignalFileResult::LoadSignalFileResult(
        const id_type project_id,
        std::vector<std::unique_ptr<Signal>> loaded_signal
) :
    project_id(project_id),
    loaded_signals(std::move(loaded_signal))
{
}

LoadSignalFileResult::~LoadSignalFileResult() noexcept = default;

LoadSignalFileResult::LoadSignalFileResult(LoadSignalFileResult&&) noexcept = default;

LoadSignalFileResult& LoadSignalFileResult::operator=(LoadSignalFileResult&&) noexcept = default;

std::vector<std::unique_ptr<Signal>> LoadSignalFileResult::take_signals() && noexcept
{
    return std::move(loaded_signals);
}

id_type LoadSignalFileResult::get_project_id() const noexcept
{
    return project_id;
}

} // namespace echomap

/**
 * @file
 *
 * NativeActionController implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-18
 */

#include "NativeActionController.hpp"

namespace echomap
{

void NativeActionController::select_project_file_impl()
{
    // TODO
    throw std::runtime_error("Not implemented");
}

void NativeActionController::complete_signal_load_impl(
        [[maybe_unused]] const std::size_t project_id,
        [[maybe_unused]] const std::size_t signal_id
)
{
    // TODO
    throw std::runtime_error("Not implemented");
}

void NativeActionController::notify_project_file_impl(
        const std::filesystem::path& path
)
{
    notify_project_file(path);
}

void NativeActionController::complete_signal_load_impl(
        const std::size_t project_id,
        const std::size_t signal_id,
        const std::filesystem::path& path
)
{
    notify_complete_signal_load(project_id, signal_id, path);
}

} // namespace echomap

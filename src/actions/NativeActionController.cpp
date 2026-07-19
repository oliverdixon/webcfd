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

void NativeActionController::register_vfs_mapping_impl(
        std::size_t project_id,
        const std::filesystem::path& external
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

void NativeActionController::notify_vfs_mapping_impl(
        const std::size_t project_id,
        const std::filesystem::path& external,
        const std::filesystem::path& internal
)
{
    notify_vfs_mapping(project_id, external, internal);
}

} // namespace echomap

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
    throw std::runtime_error("Not implemented");
}

void NativeActionController::notify_project_file_impl(
        const std::filesystem::path& path
)
{
    std::ignore = path;
    throw std::runtime_error("Not implemented");
}

} // namespace echomap

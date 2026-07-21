/**
 * @file
 *
 * RegisterVFSMappingNotification implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-20
 */

#include "RegisterVFSMappingNotification.hpp"

#include "../errors/IgnoredWarning.hpp"
#include "../objects/Project.hpp"

namespace echomap
{

RegisterVFSMappingNotification::RegisterVFSMappingNotification(
        const id_type project_id,
        std::filesystem::path external,
        std::filesystem::path internal
) :
    project_id(project_id),
    external(std::move(external)),
    internal(std::move(internal))
{
}

void RegisterVFSMappingNotification::verify_project(
        const Project* const context
) const
{
    if (context == nullptr)
        throw IgnoredWarning("Dropping RegisterVFSMappingNotification due to empty project.");

    if (context->get_id() != project_id)
        throw IgnoredWarning(
                std::format(
                        "Dropping RegisterVFSMappingNotification due to invalid project: requested {}, but have {}.",
                        project_id,
                        context->get_id()
                )
        );
}

} // namespace echomap

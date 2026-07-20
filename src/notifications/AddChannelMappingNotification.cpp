/**
 * @file
 *
 * AddChannelMappingNotification implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-20
 */

#include "AddChannelMappingNotification.hpp"

#include "../errors/IgnoredWarning.hpp"

namespace echomap
{

AddChannelMappingNotification::AddChannelMappingNotification(
        const id_type project_id,
        const id_type signal_id,
        const id_type sensor_id
) :
    project_id(project_id),
    signal_id(signal_id),
    sensor_id(sensor_id)
{
}

void AddChannelMappingNotification::verify_project(
        const Project* const context
) const
{
    if (context == nullptr)
        throw IgnoredWarning("Dropping AddChannelMappingNotification due to empty project.");

    if (context->get_id() != project_id)
        throw IgnoredWarning(
                std::format(
                        "Dropping AddChannelMappingNotification due to invalid project: requested {}, but have {}.",
                        project_id,
                        context->get_id()
                )
        );
}

} // namespace echomap

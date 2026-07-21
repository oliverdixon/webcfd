/**
 * @file
 *
 * ModifySensorPositionNotification implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-20
 */

#include "ModifySensorPositionNotification.hpp"

#include "../errors/IgnoredWarning.hpp"
#include "../objects/Project.hpp"

namespace echomap
{

ModifySensorPositionNotification::ModifySensorPositionNotification(
        const id_type project_id,
        const id_type sensor_id,
        const Position& position
) :
    project_id(project_id),
    sensor_id(sensor_id),
    position(position)
{
}

void ModifySensorPositionNotification::verify_project(
        const Project* context
) const
{
    if (context == nullptr)
        throw IgnoredWarning("Dropping ModifySensorPositionNotification due to empty project.");

    if (context->get_id() != project_id)
        throw IgnoredWarning(
                std::format(
                        "Dropping ModifySensorPositionNotification due to invalid project: requested {}, but have {}.",
                        project_id,
                        context->get_id()
                )
        );
}

} // namespace echomap

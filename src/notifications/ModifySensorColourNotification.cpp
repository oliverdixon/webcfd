/**
 * @file
 *
 * ModifySensorColourNotification implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-20
 */

#include "ModifySensorColourNotification.hpp"

#include "../errors/IgnoredWarning.hpp"
#include "../objects/Project.hpp"

namespace echomap
{

ModifySensorColourNotification::ModifySensorColourNotification(
        const id_type project_id,
        const id_type sensor_id,
        const Colour& colour
) :
    project_id(project_id),
    sensor_id(sensor_id),
    colour(colour)
{
}

void ModifySensorColourNotification::verify_project(
        const Project* const context
) const
{
    if (context == nullptr)
        throw IgnoredWarning("Dropping ModifySensorColourNotification due to empty project.");

    if (context->get_id() != project_id)
        throw IgnoredWarning(
                std::format(
                        "Dropping ModifySensorColourNotification due to invalid project: requested {}, but have {}.",
                        project_id,
                        context->get_id()
                )
        );
}

} // namespace echomap

/**
 * @file
 *
 * ModifySensorColourNotification specification
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#ifndef ECHOMAP_MODIFYSENSORCOLOURTASK_HPP
#define ECHOMAP_MODIFYSENSORCOLOURTASK_HPP

#include "../objects/IDAllocator.hpp"
#include "../utility/Colour.hpp"

namespace echomap
{

class Project;

/**
 * A notification indicating that the colour of a Sensor should be changed.
 *
 * Produced by the SensorGeometryPanel when the user requests to move the colour of a Sensor from the front-end.
 *
 * @ingroup Notifications
 */
struct ModifySensorColourNotification
{
    /**
     * Create a new ModifySensorPositionNotification to indicate that a Sensor has changed in colour.
     *
     * @param project_id The ID of the Project which owns the updated Sensor.
     * @param sensor_id The ID of the moved Sensor.
     * @param colour The new Colour of the referenced Sensor.
     */
    explicit ModifySensorColourNotification(
            id_type project_id,
            id_type sensor_id,
            const Colour& colour
    );

    /**
     * Verify that the given Project matches the intended target.
     *
     * @param context The context to which the notification will apply.
     * @throws IgnoredWarning The notification does not apply to the given context and should be ignored.
     */
    void verify_project(const Project* context) const;

    id_type project_id; /**< The ID of the Project which owns the updated Sensor. */
    id_type sensor_id;  /**< The ID of the moved Sensor. */
    Colour colour;      /**< The new Colour of the referenced Sensor. */
};

} // namespace echomap

#endif // ECHOMAP_MODIFYSENSORCOLOURTASK_HPP

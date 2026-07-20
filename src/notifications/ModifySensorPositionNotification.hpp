/**
 * @file
 *
 * ModifySensorPositionNotification specification
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#ifndef ECHOMAP_MODIFYSENSORPOSITIONNOTIFICATION_HPP
#define ECHOMAP_MODIFYSENSORPOSITIONNOTIFICATION_HPP

#include "../objects/Project.hpp"
#include "../objects/Sensor.hpp"

namespace echomap
{

/**
 * A notification indicating that the position of a Sensor should be changed.
 *
 * Produced by the SensorGeometryPanel when the user requests to move the position of a Sensor from the front-end.
 *
 * @ingroup Notifications
 */
struct ModifySensorPositionNotification
{
    /**
     * Create a new ModifySensorPositionNotification to indicate that a Sensor has moved.
     *
     * @param project_id The ID of the Project which owns the updated Sensor.
     * @param sensor_id The ID of the moved Sensor.
     * @param position The new Position of the referenced Sensor.
     */
    explicit ModifySensorPositionNotification(
            id_type project_id,
            id_type sensor_id,
            const Sensor::Position& position
    );

    /**
     * Verify that the given Project matches the intended target.
     *
     * @param context The context to which the notification will apply.
     * @throws IgnoredWarning The notification does not apply to the given context and should be ignored.
     */
    void verify_project(const Project* context) const;

    id_type project_id;        /**< The ID of the Project which owns the updated Sensor. */
    id_type sensor_id;         /**< The ID of the moved Sensor. */
    Sensor::Position position;   /**< The new Position of the referenced Sensor. */
};

} // namespace echomap

#endif // ECHOMAP_MODIFYSENSORPOSITIONNOTIFICATION_HPP

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

#include "../objects/Sensor.hpp"

namespace echomap
{

/**
 * A notification indicating that the position of a Sensor should be changed.
 *
 * @ingroup Notifications
 */
struct ModifySensorPositionNotification
{
    /**
     * Create a new ModifySensorPositionNotification to indicate that a Sensor has moved.
     *
     * @todo Qualify with project ID.
     *
     * @param sensor_id The ID of the moved Sensor.
     * @param position The new Position of the referenced Sensor.
     */
    explicit ModifySensorPositionNotification(
            const Sensor::id_type sensor_id,
            const Sensor::Position& position
    ) :
        sensor_id(sensor_id),
        position(position)
    {
    }

    Sensor::id_type sensor_id; /**< The ID of the moved Sensor. */
    Sensor::Position position; /**< The new Position of the referenced Sensor. */
};

} // namespace echomap

#endif // ECHOMAP_MODIFYSENSORPOSITIONNOTIFICATION_HPP

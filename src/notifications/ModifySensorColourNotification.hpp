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

#include "../objects/Sensor.hpp"

namespace echomap
{

/**
 * A notification indicating that the colour of a Sensor should be changed.
 *
 * @ingroup Notifications
 */
struct ModifySensorColourNotification
{
    /**
     * Create a new ModifySensorPositionNotification to indicate that a Sensor has changed in colour.
     *
     * @todo Qualify with project ID.
     *
     * @param sensor_id The ID of the moved Sensor.
     * @param colour The new Colour of the referenced Sensor.
     */
    explicit ModifySensorColourNotification(
            const Sensor::id_type sensor_id,
            const Sensor::Colour& colour
    ) :
        sensor_id(sensor_id),
        colour(colour)
    {
    }

    Sensor::id_type sensor_id; /**< The ID of the moved Sensor. */
    Sensor::Colour colour;     /**< The new Colour of the referenced Sensor. */
};

} // namespace echomap

#endif // ECHOMAP_MODIFYSENSORCOLOURTASK_HPP

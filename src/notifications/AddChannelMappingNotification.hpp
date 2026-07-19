/**
 * @file
 *
 * AddChannelMappingNotification specification
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#ifndef ECHOMAP_ADDCHANNELMAPPINGNOTIFICATION_HPP
#define ECHOMAP_ADDCHANNELMAPPINGNOTIFICATION_HPP

#include "../objects/Sensor.hpp"
#include "../objects/Signal.hpp"

namespace echomap
{

/**
 * A notification indicating that a new channel mapping should be established.
 *
 * @todo Qualify with Project ID.
 */
struct AddChannelMappingNotification
{
    Signal::id_type signal_id; /**< The ID of the participating Signal. */
    Sensor::id_type sensor_id; /**< The ID of the participating Sensor. */
};

} // namespace echomap

#endif // ECHOMAP_ADDCHANNELMAPPINGNOTIFICATION_HPP

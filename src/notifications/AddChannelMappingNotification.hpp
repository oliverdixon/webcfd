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
#include "../objects/Project.hpp"

namespace echomap
{

/**
 * A notification indicating that a new channel mapping should be established.
 *
 * @ingroup Notifications
 */
struct AddChannelMappingNotification
{
    /**
     * Create a new AddChannelMappingNotification to indicate that the Signal and Sensor within a Project should be
     *  mapped.
     *
     * @param project_id The ID of the Project detaining the Signal and Sensor.
     * @param signal_id The ID of the participating Signal.
     * @param sensor_id The ID of the participating Sensor.
     */
    AddChannelMappingNotification(
            const Project::id_type project_id,
            const Signal::id_type signal_id,
            const Sensor::id_type sensor_id
    ) :
        project_id(project_id),
        signal_id(signal_id),
        sensor_id(sensor_id)
    {
    }

    Project::id_type project_id; /** The ID of the Project detaining the Signal and Sensor. */
    Signal::id_type signal_id; /**< The ID of the participating Signal. */
    Sensor::id_type sensor_id; /**< The ID of the participating Sensor. */
};

} // namespace echomap

#endif // ECHOMAP_ADDCHANNELMAPPINGNOTIFICATION_HPP

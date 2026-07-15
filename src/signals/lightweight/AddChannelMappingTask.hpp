/**
 * @file
 *
 * AddChannelMappingTask specification
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#ifndef ECHOMAP_ADDCHANNELMAPPINGTASK_HPP
#define ECHOMAP_ADDCHANNELMAPPINGTASK_HPP

#include "../../objects/Sensor.hpp"
#include "../../objects/Signal.hpp"

namespace echomap
{

/**
 * A lightweight task indicating that a new channel mapping should be established between the Signal and Sensor with
 * given IDs, in the context of the active Project.
 */
struct AddChannelMappingTask
{
    Signal::id_type signal_id;
    Sensor::id_type sensor_id;
};

} // namespace echomap

#endif // ECHOMAP_ADDCHANNELMAPPINGTASK_HPP

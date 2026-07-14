/**
 * @file
 *
 * ModifySensorPositionTask specification
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#ifndef ECHOMAP_MODIFYSENSORPOSITIONTASK_HPP
#define ECHOMAP_MODIFYSENSORPOSITIONTASK_HPP

#include "../../objects/Sensor.hpp"

namespace echomap
{

/**
 * A lightweight task indicating that the position of Sensor identified by the given ID, in the active Project, should
 * be changed to the detained position.
 */
class ModifySensorPositionTask
{
public:
    Sensor::id_type sensor_id;
    Sensor::Position position;
};

} // namespace echomap

#endif // ECHOMAP_MODIFYSENSORPOSITIONTASK_HPP

//
// Created by owd on 24/06/2026.
//

#include "Sensor.hpp"

namespace WebCFD
{

template <> constexpr std::string Object<Sensor>::class_name = "Sensor";

Sensor::Sensor(
        const std::string_view name
) :
    Object(name)
{
}

Sensor::Sensor(
        const Position position,
        std::string_view name
) :
    position(position)
{
}

Sensor::Sensor(
        const Sensor& old_sensor
) :
    Object(CopyTag{}, old_sensor)
{
}

Sensor::Sensor(
        const Sensor& old_sensor,
        const std::string_view new_name
) :
    Object(CopyTag{}, old_sensor, new_name)
{
}

} // namespace WebCFD

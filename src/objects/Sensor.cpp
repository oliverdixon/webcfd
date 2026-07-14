//
// Created by owd on 24/06/2026.
//

#include "Sensor.hpp"

namespace echomap
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
        const std::string_view name
) :
    Object(name),
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

void Sensor::set_colour(
        const Colour new_colour
) noexcept
{
    colour = new_colour;
}

void Sensor::set_position(
        const Position new_position
) noexcept
{
    position = new_position;
}

} // namespace echomap

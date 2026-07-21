//
// Created by owd on 24/06/2026.
//

#ifndef ECHOMAP_SENSOR_HPP
#define ECHOMAP_SENSOR_HPP

#include "../utility/Colour.hpp"
#include "../utility/Position.hpp"
#include "Object.hpp"

namespace echomap
{

class Sensor : public Object<Sensor>
{
public:
    Position position{};
    Colour colour{};

    explicit Sensor(std::string_view name = {});

    explicit Sensor(Position position, std::string_view name = {});

    Sensor(const Sensor& old_sensor);
    Sensor(const Sensor& old_sensor, std::string_view new_name);

    void set_colour(Colour new_colour) noexcept;
    void set_position(Position new_position) noexcept;
};

template <> constexpr std::string_view Object<Sensor>::class_name = "Sensor";

} // namespace echomap

#endif // ECHOMAP_SENSOR_HPP

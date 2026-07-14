//
// Created by owd on 24/06/2026.
//

#ifndef ECHOMAP_SENSOR_HPP
#define ECHOMAP_SENSOR_HPP

#include "Object.hpp"

namespace echomap
{

class Sensor : public Object<Sensor>
{
public:
    struct Position
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    } position{};

    struct Colour
    {
        float r = 1.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 1.0f;
    } colour{};

    explicit Sensor(std::string_view name = {});

    explicit Sensor(Position position, std::string_view name = {});

    Sensor(const Sensor& old_sensor);
    Sensor(const Sensor& old_sensor, std::string_view new_name);


    void set_colour(Colour new_colour) noexcept;
    void set_position(Position new_position) noexcept;
};

} // namespace echomap

#endif // ECHOMAP_SENSOR_HPP

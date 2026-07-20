//
// Created by owd on 25/06/2026.
//

#include "Project.hpp"

#include <format>

#include "Sensor.hpp"

namespace echomap
{

Project::Project(
        const std::string_view project_name
) :
    Object(project_name)
{
}

const Signal* Project::add_signal(
        std::shared_ptr<Signal>&& signal
)
{
    const auto [it, success] = signals.emplace(signal->get_id(), std::move(signal));
    if (!success)
        return nullptr;

    return it->second ? it->second.get() : nullptr;
}

const Sensor* Project::add_sensor(
        std::unique_ptr<Sensor>&& sensor
)
{
    const auto [it, success] = sensors.emplace(sensor->get_id(), std::move(sensor));
    if (!success)
        return nullptr;

    return it->second ? it->second.get() : nullptr;
}

void Project::add_association(
        const Signal& signal,
        const Sensor& sensor
)
{
    if (!signals.contains(signal.get_id()))
        throw std::runtime_error(std::format("{} does not exist in the project.", signal.get_name()));

    if (!sensors.contains(sensor.get_id()))
        throw std::runtime_error(std::format("{} does not exist in the project.", sensor.get_name()));

    if (!requested_channel_mapping.emplace(signal.get_id(), sensor.get_id()).second)
        throw std::runtime_error(
                std::format(
                        "Could not associate {} with {}: a component is already mapped.",
                        signal.get_name(),
                        sensor.get_name()
                )
        );
}

void Project::add_association(
        const id_type signal_id,
        const id_type sensor_id
)
{
    if (!requested_channel_mapping.emplace(signal_id, sensor_id).second)
        throw std::runtime_error(
                std::format(
                        "Could not associate Signal with ID {} to Sensor with ID {}: a component is already mapped.",
                        signal_id,
                        sensor_id
                )
        );
}

std::size_t Project::get_sensors_count() const noexcept
{
    return sensors.size();
}

ImPlot3DPoint Project::get_sensor_point(
        const int idx,
        const void* const project_instance
) noexcept
{
    const auto* const project_ptr = static_cast<const Project*>(project_instance);
    const auto [x, y, z] = project_ptr->sensors.values()[idx]->position;

    return { x, y, z };
}

Sensor& Project::get_mutable_sensor(
        const id_type sensor_id
)
{
    const auto sensor_it = sensors.find(sensor_id);
    if (sensor_it == sensors.end())
        throw std::runtime_error(std::format("No Sensor with ID {} belongs to {}.", sensor_id, get_name()));

    return *sensor_it->second;
}

std::optional<std::pair<
        const Signal&,
        const Sensor&>>
Project::resolve_pair(
        const id_type signal_id,
        const id_type sensor_id
) const
{
    /*
     * For the exception messages here, the best we can manage is the ID. Logging the display names would require
     * resolving the full Object, which we can't do if we can't find the thing!
     */

    const auto signal_it = signals.find(signal_id);
    if (signal_it == signals.end())
        return {};

    const auto sensor_it = sensors.find(sensor_id);
    if (sensor_it == sensors.end())
        return {};

    assert(signal_it->second != nullptr);
    assert(sensor_it->second != nullptr);

    return {{*signal_it->second, *sensor_it->second}};
}

} // namespace echomap

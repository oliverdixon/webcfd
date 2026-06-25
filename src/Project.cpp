//
// Created by owd on 25/06/2026.
//

#include "Project.hpp"

#include <format>

namespace WebCFD
{

template <> constexpr std::string Object<Project>::class_name = "Project";

Project::Project(
        const std::string_view project_name
) :
    Object(project_name)
{
    // TODO remove. Adding data for testing...

    const auto signal_a = add_signal(std::make_unique<Signal>());
    const auto signal_b = add_signal(std::make_unique<Signal>("Signal B"));
    const auto signal_c = add_signal(std::make_unique<Signal>("Signal C"));

    assert(signal_a && signal_b && signal_c);

    const auto sensor_a = add_sensor(std::make_unique<Sensor>("Sensor A"));
    const auto sensor_b = add_sensor(std::make_unique<Sensor>("Sensor B"));
    const auto sensor_c = add_sensor(std::make_unique<Sensor>());

    assert(sensor_a && sensor_b && sensor_c);

    add_association(*signal_a, *sensor_b);
}

const Signal* Project::add_signal(
        std::unique_ptr<Signal>&& signal
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

    if (!channel_mapping.emplace(signal.get_id(), sensor.get_id()).second)
        throw std::runtime_error(
                std::format(
                        "Could not associate {} with {}: a component is already mapped.",
                        signal.get_name(),
                        sensor.get_name()
                )
        );
}

bool Project::are_signals_stored() const noexcept
{
    return !signals.empty();
}

bool Project::are_sensors_stored() const noexcept
{
    return !sensors.empty();
}

bool Project::are_associations_defined() const noexcept
{
    return !channel_mapping.empty();
}

std::pair<
        const Signal&,
        const Sensor&>
Project::resolve_pair(
        const Signal::id_type signal_id,
        const Sensor::id_type sensor_id
) const
{
    /*
     * For the exception messages here, the best we can manage is the ID. Logging the display names would require
     * resolving the full Object, which we can't do if we can't find the thing!
     */

    const auto signal_it = signals.find(signal_id);
    if (signal_it == signals.end())
        throw std::runtime_error(
                std::format("{} with ID {} does not exist in the project.", Object<Signal>::get_class_name(), signal_id)
        );

    const auto sensor_it = sensors.find(sensor_id);
    if (sensor_it == sensors.end())
        throw std::runtime_error(
                std::format("{} with ID {} does not exist in the project.", Object<Sensor>::get_class_name(), sensor_id)
        );

    assert(signal_it->second);
    assert(sensor_it->second);

    return {*signal_it->second, *sensor_it->second};
}

} // namespace WebCFD

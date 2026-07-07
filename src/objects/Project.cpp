//
// Created by owd on 25/06/2026.
//

#include "Project.hpp"

#include <format>

#include "Sensor.hpp"
#include "persistence/JSONSerialiser.hpp"

namespace WebCFD
{

template <> constexpr std::string Object<Project>::class_name = "Project";

Project::Project(
        const std::string_view project_name
) :
    Object(project_name)
{
    // TODO remove. Adding data for testing...

    auto file_signals = SignalFactory::load_wave_file("../audio/4channel.wav");

    std::vector<const Signal *> loaded_signals;
    std::vector<const Sensor *> loaded_sensors;

    loaded_signals.reserve(file_signals.size());
    loaded_sensors.reserve(file_signals.size());

    for (auto&& signal : file_signals) {
        loaded_signals.push_back(add_signal(std::move(signal)));
        loaded_sensors.push_back(add_sensor(std::make_unique<Sensor>()));

        assert(loaded_signals.back());
        assert(loaded_sensors.back());
    }

    for (auto [signal, sensor] : std::views::zip(observe_signals(), std::views::reverse(observe_sensors())))
        add_association(signal, sensor);

    JSONSerialiser serialiser;
    std::cout << JSONSerialiser::pretty_print(serialiser.serialise(*this)) << std::endl;
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

std::size_t Project::get_signal_count() const noexcept
{
    return signals.size();
}

std::size_t Project::get_sensors_count() const noexcept
{
    return sensors.size();
}

std::size_t Project::get_associations_count() const noexcept
{
    return channel_mapping.size();
}

ImPlot3DPoint Project::get_sensor_point(
        const int idx,
        const void* const project_instance
) noexcept
{
    const auto project_ptr = static_cast<const Project *>(project_instance);
    const auto [x, y, z] = project_ptr->sensors.values()[idx]->position;

    return { x, y, z };
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

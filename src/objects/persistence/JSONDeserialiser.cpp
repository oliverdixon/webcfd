//
// Created by owd on 07/07/2026.

#include "JSONDeserialiser.hpp"

#include "../Project.hpp"
#include "../Signal.hpp"
#include "../SignalFactory.hpp"

/**
 * Free helper functions for simdjson customisation points.
 */
namespace
{

template <typename simdjson_value>
auto get_root(
        simdjson::ondemand::object& root,
        simdjson_value& value
)
{
    auto error = value.get_object().get(root);
    if (error)
        return error;

    // Schema version check.
    static constexpr unsigned int expected_schema_version = 1;
    unsigned int actual_schema_version = 0;

    if ((error = root["schema_version"].get(actual_schema_version)))
        return error;

    if (actual_schema_version != expected_schema_version)
        throw std::runtime_error(
                std::format(
                        "Project file was saved in an incompatible version {} (expected {}).",
                        actual_schema_version,
                        expected_schema_version
                )
        );

    return simdjson::SUCCESS;
}

auto get_metadata(
        simdjson::ondemand::object& root,
        echomap::Project& project
)
{
    simdjson::ondemand::object metadata;
    auto error = root["metadata"].get_object().get(metadata);
    if (error)
        return error;

    std::string project_name;
    if ((error = metadata["name"].get(project_name)))
        return error;

    project.set_name(project_name);
    return simdjson::SUCCESS;
}

auto get_signals(
        simdjson::ondemand::object& root,
        echomap::Project& project,
        std::unordered_map<
                std::string_view,
                echomap::Signal::id_type>& loaded
)
{
    std::vector<std::unique_ptr<echomap::Signal>> signals;
    if (const auto error = root["signals"].get(signals))
        return error;

    /*
     * This map associates paths of wave files with slot vectors for the corresponding file. Entries are constructed for
     * each Signal whose source is a "filesystem" type.
     *
     * Each slot vector provides a mapping between the channel index and the destination Signal object.
     *
     * That is, the first slot stores the destination Signal for the Channel #1, the second for Channel #2, etc.
     */
    std::unordered_map<std::string, std::vector<echomap::Signal*>> slots;

    for (const auto& signal : signals) {
        if (!loaded.emplace(signal->get_name(), signal->get_id()).second)
            throw std::runtime_error(std::format("Project contains duplicate signal {}.", signal->get_name()));

        if (signal->observe_source().has_value()) {
            /*
             * If the Signal has a Source, it comes from the filesystem. Register the Signal as a destination for its
             * source file for its channel number.
             */
            auto& slot_vector = slots[signal->observe_source()->path.c_str()];
            const auto channel_num = signal->observe_source()->channel;

            // Reminder: channel numbers are 1-based.
            if (channel_num > slot_vector.size())
                slot_vector.resize(channel_num, nullptr);
            else if (slot_vector[channel_num] != nullptr)
                throw std::runtime_error(
                        std::format(
                                "Both signals {} and {} have requested the same channel {} from {}.",
                                slot_vector[channel_num]->get_name(),
                                signal->get_name(),
                                channel_num,
                                signal->observe_source()->path.c_str()
                        )
                );

            slot_vector[channel_num - 1] = signal.get();
        }
    }

    // Once we've constructed all the signals, we can finish loading the filesystem-sourced signals.
    for (const auto& [file_path, slot_vector] : slots)
        echomap::SignalFactory::load_wave_file(file_path.c_str(), slot_vector);

    // Now we can give the fully constructed signals to the project.
    for (auto&& signal : signals)
        project.add_signal(std::move(signal));

    return simdjson::SUCCESS;
}

auto get_sensors(
        simdjson::ondemand::object& root,
        echomap::Project& project,
        std::unordered_map<
                std::string_view,
                echomap::Sensor::id_type>& loaded
)
{
    std::vector<std::unique_ptr<echomap::Sensor>> sensors;
    if (const auto error = root["sensors"].get(sensors))
        return error;

    for (auto&& sensor : sensors) {
        if (!loaded.emplace(sensor->get_name(), sensor->get_id()).second)
            throw std::runtime_error(std::format("Project contains duplicate sensor {}.", sensor->get_name()));
        project.add_sensor(std::move(sensor));
    }

    return simdjson::SUCCESS;
}

auto get_mappings(
        simdjson::ondemand::object& root,
        echomap::Project& project,
        const std::unordered_map<
                std::string_view,
                echomap::Signal::id_type>& signals,
        const std::unordered_map<
                std::string_view,
                echomap::Sensor::id_type>& sensors
)
{
    simdjson::ondemand::array mappings;
    auto error = root["mappings"].get_array().get(mappings);
    if (error)
        return error;

    for (auto mapping : mappings) {
        simdjson::ondemand::object mapping_obj;
        if ((error = mapping.get_object().get(mapping_obj)))
            return error;

        std::string_view signal_name;
        std::string_view sensor_name;

        if ((error = mapping_obj["signal"].get(signal_name)))
            return error;

        if ((error = mapping_obj["sensor"].get(sensor_name)))
            return error;

        const auto signal_it = signals.find(signal_name);
        if (signal_it == signals.end())
            throw std::runtime_error(std::format("Channel mapping referred to non-existent signal {}.", signal_name));

        const auto sensor_it = sensors.find(sensor_name);
        if (sensor_it == sensors.end())
            throw std::runtime_error(std::format("Channel mapping referred to non-existent sensor {}.", sensor_name));

        project.add_association(signal_it->second, sensor_it->second);
    }

    return simdjson::SUCCESS;
}

void verify_sample_count(
        std::uint64_t reported_sample_count,
        const echomap::Signal& signal
)
{
    if (reported_sample_count != signal.get_sample_count())
        throw std::runtime_error(
                std::format(
                        "Embedded signal {} reported the incorrect number of samples: claimed {}, but received {}.",
                        signal.get_name(),
                        reported_sample_count,
                        signal.get_sample_count()
                )
        );
}

template <typename ValueT>
simdjson::error_code get_or_default(
        simdjson::ondemand::object& obj,
        const std::string_view key,
        ValueT& out,
        const ValueT& fallback
)
{
    auto field = obj[key];

    if (field.error() == simdjson::NO_SUCH_FIELD) {
        out = fallback;
        return simdjson::SUCCESS;
    }

    return field.get(out);
}

auto get_uniformly_sampled_signal_source(
        simdjson::ondemand::object& source,
        echomap::Signal& signal
)
{
    std::uint64_t reported_sample_count;
    auto error = source["sample_count"].get(reported_sample_count);
    if (error)
        return error;

    std::size_t sample_rate;
    if ((error = source["sample_rate"].get(sample_rate)))
        return error;
    signal.set_sample_rate(sample_rate);

    float time_offset;
    if ((error = source["time_offset"].get(time_offset)))
        return error;
    signal.set_time_offset(time_offset);

    simdjson::ondemand::array samples;
    if ((error = source["samples"].get_array().get(samples)))
        return error;

    for (auto sample_wrapper : samples) {
        float amplitude;
        if ((error = sample_wrapper.get(amplitude)))
            return error;
        signal.emplace_sample(amplitude);
    }

    verify_sample_count(reported_sample_count, signal);
    return error;
}

auto get_variably_sampled_signal_source(
        simdjson::ondemand::object& source,
        echomap::Signal& signal
)
{
    std::uint64_t reported_sample_count;
    auto error = source["sample_count"].get(reported_sample_count);
    if (error)
        return error;

    std::size_t sample_rate;
    if ((error = get_or_default(source, "sample_rate", sample_rate, std::size_t{0})))
        return error;
    signal.set_sample_rate(sample_rate);

    float time_offset;
    if ((error = get_or_default(source, "time_offset", time_offset, 0.0f)))
        return error;
    signal.set_time_offset(time_offset);

    simdjson::ondemand::array samples;
    if ((error = source["samples"].get_array().get(samples)))
        return error;

    for (auto sample_wrapper : samples) {
        simdjson::ondemand::object sample_obj;
        if ((error = sample_wrapper.get_object().get(sample_obj)))
            return error;
        echomap::Signal::Sample sample_data; // NOLINT(*-pro-type-member-init) - Immediately initialised.
        if ((error = sample_obj["time"].get(sample_data.time)))
            return error;
        if ((error = sample_obj["amplitude"].get(sample_data.amplitude)))
            return error;
        signal.emplace_sample(sample_data.time, sample_data.amplitude);
    }

    verify_sample_count(reported_sample_count, signal);
    return error;
}

} // namespace

namespace simdjson
{

template <typename simdjson_value>
auto tag_invoke(
        deserialize_tag,
        simdjson_value& value,
        echomap::Project& project
)
{
    ondemand::object root;
    auto error = get_root(root, value);
    if (error)
        return error;

    // Metadata
    if ((error = get_metadata(root, project)))
        return error;

    // Signals
    std::unordered_map<std::string_view, echomap::Signal::id_type> signal_ids;
    if ((error = get_signals(root, project, signal_ids)))
        return error;

    // Sensors
    std::unordered_map<std::string_view, echomap::Sensor::id_type> sensor_ids;
    if ((error = get_sensors(root, project, sensor_ids)))
        return error;

    // Channel Map
    if ((error = get_mappings(root, project, signal_ids, sensor_ids)))
        return error;

    return SUCCESS;
}

template <typename simdjson_value>
auto tag_invoke(
        deserialize_tag,
        simdjson_value& value,
        echomap::Signal& signal
)
{
    ondemand::object root;
    auto error = value.get_object().get(root);
    if (error)
        return error;

    std::string_view name;
    if ((error = root["name"].get(name)))
        return error;
    signal.set_name(name);

    ondemand::object source;
    if ((error = root["source"].get_object().get(source)))
        return error;

    std::string_view kind;
    if ((error = source["kind"].get(kind)))
        return error;

    if (kind == "filesystem") {
        // Signals sourced from the file system are not loaded by the parser; we just populate the source information.

        std::string_view path;
        if ((error = source["path"].get(path)))
            return error;

        std::size_t channel_num;
        if ((error = source["channel"].get(channel_num)))
            return error;

        signal.set_source(path, channel_num);
    } else if (kind == "embeddedUniform") {
        // Embedded signals with uniform sampling are constructed by the parser with the mandatory timing information.
        if ((error = get_uniformly_sampled_signal_source(source, signal)))
            return error;
    } else if (kind == "embeddedVariable") {
        // Embedded signals with variable sampling are constructed by the parser with the optional timing information.
        if ((error = get_variably_sampled_signal_source(source, signal)))
            return error;
    } else
        throw std::runtime_error(std::format("Signal {} specifies unknown source kind \"{}\".", name, kind));

    return SUCCESS;
}

template <typename simdjson_value>
auto tag_invoke(
        deserialize_tag,
        simdjson_value& value,
        echomap::Sensor& sensor
)
{
    ondemand::object root;
    auto error = value.get_object().get(root);
    if (error)
        return error;

    std::string_view name;
    if ((error = root["name"].get(name)))
        return error;
    sensor.set_name(name);

    ondemand::object position;
    if ((error = root["position"].get_object().get(position)))
        return error;

    if ((error = position["x"].get(sensor.position.x)))
        return error;
    if ((error = position["y"].get(sensor.position.y)))
        return error;
    if ((error = position["z"].get(sensor.position.z)))
        return error;

    ondemand::object colour;
    if ((error = root["colour"].get_object().get(colour)))
        return error;

    if ((error = colour["r"].get(sensor.colour.r)))
        return error;
    if ((error = colour["g"].get(sensor.colour.g)))
        return error;
    if ((error = colour["b"].get(sensor.colour.b)))
        return error;
    if ((error = colour["a"].get(sensor.colour.a)))
        return error;

    return SUCCESS;
}

} // namespace simdjson

namespace echomap
{

std::unique_ptr<Project> JSONDeserialiser::deserialise_project(
        const std::string_view path
)
{
    const auto json = simdjson::padded_string::load(path);
    auto doc = parser.iterate(json);
    auto project = std::make_unique<Project>();

    if (const auto error = doc.get(*project); error)
        return nullptr;

    return std::move(project);
}

} // namespace echomap

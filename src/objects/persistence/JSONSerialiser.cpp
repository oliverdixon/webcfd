//
// Created by owd on 07/07/2026.
//

#include "JSONSerialiser.hpp"

#include "../Project.hpp"

namespace simdjson
{

template<typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder, const WebCFD::Project& project)
{
    static constexpr unsigned int schema_version = 1;
    bool first_element = true;

    builder.start_object();
    builder.append_key_value("schema_version", schema_version);
    builder.append_comma();

    // Project Metadata
    builder.escape_and_append_with_quotes("metadata");
    builder.append_colon();
    builder.start_object();
    builder.append_key_value("id", project.get_id());
    builder.append_comma();
    builder.append_key_value("name", project.get_name());
    builder.append_comma();
    builder.append_key_value("description", "UNDETERMINED");
    builder.end_object();
    builder.append_comma();

    // Signals
    builder.escape_and_append_with_quotes("signals");
    builder.append_colon();
    builder.start_array();
    for (const auto& signal : project.observe_signals()) {
        if (!first_element)
            builder.append_comma();

        builder.append(signal);
        first_element = false;
    }
    first_element = true;
    builder.end_array();
    builder.append_comma();

    // Sensors
    builder.escape_and_append_with_quotes("sensors");
    builder.append_colon();
    builder.start_array();
    for (const auto& sensor : project.observe_sensors()) {
        if (!first_element)
            builder.append_comma();

        builder.append(sensor);
        first_element = false;
    }
    first_element = true;
    builder.end_array();
    builder.append_comma();

    // Channel Mappings
    builder.escape_and_append_with_quotes("mappings");
    builder.append_colon();
    builder.start_array();
    for (const auto [sensor, signal] : project.observe_associations()) {
        if (!first_element)
            builder.append_comma();

        builder.start_object();
        builder.append_key_value("sensor_id", sensor.get_id());
        builder.append_comma();
        builder.append_key_value("signal_id", signal.get_id());
        builder.end_object();

        first_element = false;
    }

    builder.end_array();
    builder.end_object();
}

template<typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder, const WebCFD::Sensor& sensor)
{
    builder.start_object();

    // Sensor Metadata
    builder.append_key_value("id", sensor.get_id());
    builder.append_comma();
    builder.append_key_value("name", sensor.get_name());
    builder.append_comma();

    // Position
    builder.escape_and_append_with_quotes("position");
    builder.append_colon();
    builder.start_object();
    builder.append_key_value("x", sensor.position.x);
    builder.append_comma();
    builder.append_key_value("y", sensor.position.y);
    builder.append_comma();
    builder.append_key_value("z", sensor.position.z);
    builder.end_object();
    builder.append_comma();

    // Colour
    builder.escape_and_append_with_quotes("colour");
    builder.append_colon();
    builder.start_object();
    builder.append_key_value("r", sensor.colour.r);
    builder.append_comma();
    builder.append_key_value("g", sensor.colour.g);
    builder.append_comma();
    builder.append_key_value("b", sensor.colour.b);
    builder.append_comma();
    builder.append_key_value("a", sensor.colour.a);
    builder.end_object();

    builder.end_object();
}

template<typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder, const WebCFD::Signal& signal)
{
    builder.start_object();

    // Signal Metadata
    builder.append_key_value("id", signal.get_id());
    builder.append_comma();
    builder.append_key_value("name", signal.get_name());
    builder.append_comma();

    // Source
    builder.escape_and_append_with_quotes("source");
    builder.append_colon();
    builder.start_object();

    // ... for clean external sources, forward file path and channel metadata.
    if (signal.observe_source().has_value() && !signal.observe_source()->dirty) {
        const auto& source = *signal.observe_source();
        builder.append_key_value("kind", "filesystem");
        builder.append_comma();
        builder.append_key_value("path", std::filesystem::absolute(source.path).c_str());
        builder.append_comma();
        builder.append_key_value("channel", source.channel);
    }

    // ... for embedded or dirty sources, append samples in situ.
    else {
        builder.append_key_value("kind", "embedded");
        builder.append_comma();
        builder.append_key_value("sample_rate_hz", signal.get_sample_count());
        builder.append_comma();
        builder.escape_and_append_with_quotes("samples");
        builder.start_array();

        bool first_element = true;
        for (const auto sample : signal) {
            if (!first_element)
                builder.append_comma();

            builder.start_object();
            builder.append_key_value("time", sample.time);
            builder.append_comma();
            builder.append_key_value("amplitude", sample.amplitude);
            builder.end_object();

            first_element = false;
        }

        builder.end_array();
    }

    builder.end_object();
    builder.end_object();
}

}

namespace WebCFD
{

std::string_view JSONSerialiser::serialise(
        const Project& project
)
{
    sb.clear();
    sb.append(project);
    return sb.view();
}

std::string_view JSONSerialiser::serialise(
        const Signal& signal
)
{
    sb.clear();
    sb.append(signal);
    return sb.view();
}

std::string_view JSONSerialiser::serialise(
        const Sensor& sensor
)
{
    sb.clear();
    sb.append(sensor);
    return sb.view();
}

std::string JSONSerialiser::pretty_print(
        const std::string_view data
)
{
    // ReSharper disable once CppLocalVariableMayBeConst
    simdjson::fractured_json_options options{};

    return simdjson::fractured_json_string(data, options);
}

} // namespace WebCFD

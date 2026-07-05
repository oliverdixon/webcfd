/**
 * @file
 * @brief WebCFD main viewport panel implementation
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#include "ViewportPanel.hpp"

#include <format>
#include <algorithm>

namespace WebCFD
{

ViewportPanel::ViewportPanel(
        Project* const initial_project
) :
    active_project(initial_project)
{
    plotting_spec_2d.Stride = sizeof(Signal::Sample);
}

const char* ViewportPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void ViewportPanel::draw() noexcept
{
    if (ImGui::Begin(panel_name.c_str(), nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar)) {
        if (ImGui::BeginTabBar("##ViewportTabBar")) {
            if (active_project != nullptr && active_project->get_sensors_count() != sensor_colours.size()) {
                // Ensure that we maintain the correct number of colours for the current number of sensors.
                const auto size_before = sensor_colours.size();
                sensor_colours.resize(active_project->get_sensors_count());

                // Initialise the new colour entries to the default value.
                if (size_before == 0)
                    std::ranges::fill(sensor_colours, default_sensor_colour);
                else if (size_before < sensor_colours.size())
                    for (std::size_t tail_idx = size_before - 1; tail_idx < sensor_colours.size(); ++tail_idx)
                        sensor_colours[tail_idx] = default_sensor_colour;

                // ... and update the plotting specification in case the resize invalidated pointers.
                plotting_spec_3d.MarkerFillColors = &*sensor_colours.begin();
                plotting_spec_3d.MarkerLineColors = plotting_spec_3d.MarkerFillColors;
            }

            draw_signal_waveforms();
            draw_sensor_geometry();
            draw_channel_mappings();

            if (ImGui::BeginTabItem("Localisation")) {
                // TODO...
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }

    error_modal.draw();
    ImGui::End();
}

void ViewportPanel::draw_signal_waveforms() noexcept
{
    if (!ImGui::BeginTabItem("Signal Waveforms"))
        return;

    if (active_project->get_signal_count() > 0 && ImPlot::BeginAlignedPlots("##WaveformAlignedGroup")) {
        ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        for (const auto& signal : active_project->observe_signals())
            if (const auto downsampled = get_downsampled_signal(signal); downsampled == nullptr)
                ImGui::Text("Could not downsample %s due to system error.", signal.get_imgui_name());
            else if (ImPlot::BeginPlot(downsampled->get_imgui_name())) {
                ImPlot::SetupAxisLinks(ImAxis_X1, &waveform_bounding_cache.X.Min, &waveform_bounding_cache.X.Max);
                ImPlot::SetupAxisLinks(ImAxis_Y1, &waveform_bounding_cache.Y.Min, &waveform_bounding_cache.Y.Max);
                ImPlot::PlotLine(
                        "",
                        &downsampled->begin()->time,
                        &downsampled->begin()->amplitude,
                        static_cast<int>(downsampled->get_sample_count()),
                        plotting_spec_2d
                );

                ImPlot::EndPlot();
            }

        ImPlot::EndAlignedPlots();
        ImPlot::PopStyleColor();
    }

    ImGui::EndTabItem();
}

void ViewportPanel::draw_sensor_geometry() noexcept
{
    if (!ImGui::BeginTabItem("Sensor Geometry"))
        return;

    if (!active_project->get_sensors_count())
        ImGui::Text("No sensors are loaded.");

    ImGui::SeparatorText("Geometry Summary");
    if (ImGui::BeginTable("##GeometrySummary", 5, table_flags)) {
        ImGui::TableSetupColumn("##SensorColourColumn", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Sensor", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("X Position", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Y Position", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Z Position", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        std::size_t row_idx = 0;

        for (auto& sensor : active_project->mutate_sensors()) {
            ImGui::PushID(sensor.get_id());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            const ImVec4 colour = ImGui::ColorConvertU32ToFloat4(sensor_colours[row_idx]);
            float rgb[4] = {colour.x, colour.y, colour.z, colour.w};
            if (ImGui::ColorEdit4("##colour", rgb, ImGuiColorEditFlags_NoInputs))
                sensor_colours[row_idx] = IM_COL32(
                        static_cast<int>(rgb[0] * 255.0f),
                        static_cast<int>(rgb[1] * 255.0f),
                        static_cast<int>(rgb[2] * 255.0f),
                        static_cast<int>(rgb[3] * 255.0f)
                );

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            ImGui::Text("%s", sensor.get_imgui_name());

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            ImGui::InputFloat("##x", &sensor.position.x, 0.01f, 1.0f);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            ImGui::InputFloat("##y", &sensor.position.y, 0.01f, 1.0f);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            ImGui::InputFloat("##z", &sensor.position.z, 0.01f, 1.0f);

            ImGui::PopID();

            ++row_idx;
        }

        ImGui::EndTable();
    }

    ImGui::SeparatorText("Geometry Plot");
    ImPlot3D::PushStyleColor(ImPlot3DCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    const auto avail_size = ImGui::GetContentRegionAvail();
    const float aspect_ratio = avail_size.x / avail_size.y;

    if (ImPlot3D::BeginPlot("##SensorGeometryPlot", ImVec2(-std::numeric_limits<float>::min(), avail_size.y))) {
        ImPlot3D::SetupBoxScale(aspect_ratio, 1.0f, 1.0f);
        ImPlot3D::SetupAxes("X", "Y", "Z");
        ImPlot3D::PlotScatterG(
                "",
                Project::get_sensor_point,
                active_project,
                static_cast<int>(active_project->get_sensors_count()),
                plotting_spec_3d
        );

        ImPlot3D::EndPlot();
    }

    ImPlot3D::PopStyleColor();
    ImGui::EndTabItem();
}

void ViewportPanel::draw_channel_mappings() noexcept
{
    if (!ImGui::BeginTabItem("Channel Mapping"))
        return;

    ImGui::SeparatorText("Create Channel Mapping");
    draw_new_channel_mapping();

    // If a new mapping has been fully described, add it and prompt for another.
    if (new_mapping_cache.selected_signal != nullptr && new_mapping_cache.selected_sensor != nullptr) {
        try {
            active_project->add_association(*new_mapping_cache.selected_signal, *new_mapping_cache.selected_sensor);
        } catch (const std::runtime_error& exception) {
            error_modal.raise_error("Cannot add channel mapping.", exception);
        }

        new_mapping_cache.selected_signal = nullptr;
        new_mapping_cache.selected_sensor = nullptr;
    }

    ImGui::SeparatorText("Existing Channel Mapping");
    draw_existing_channel_mapping();

    ImGui::EndTabItem();
}

void ViewportPanel::draw_new_channel_mapping() noexcept
{
    if (ImGui::BeginTable("##NewChannelMapping", 2, table_flags)) {
        ImGui::TableSetupColumn("Signal", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Sensor", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // Prompt for the associated signal.
        ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
        if (ImGui::BeginCombo(
                    "##NewAssociationSignal",
                    new_mapping_cache.selected_signal == nullptr ? "Select signal..."
                                                                 : new_mapping_cache.selected_signal->get_imgui_name(),
                    0
            )) {
            for (const auto& signal : active_project->observe_signals()) {
                const bool is_selected = new_mapping_cache.selected_signal == nullptr
                                                 ? false
                                                 : signal == *new_mapping_cache.selected_signal;

                // Checks if something has changed (thus current value needs updating).
                if (ImGui::Selectable(signal.get_imgui_name(), is_selected))
                    new_mapping_cache.selected_signal = &signal;

                // Checks if the current item is selected.
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::TableNextColumn();

        // Prompt for the associated sensor.
        ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
        if (ImGui::BeginCombo(
                    "##NewAssociationSensor",
                    new_mapping_cache.selected_sensor == nullptr ? "Select sensor..."
                                                                 : new_mapping_cache.selected_sensor->get_imgui_name(),
                    0
            )) {
            for (const auto& sensor : active_project->observe_sensors()) {
                const bool is_selected = new_mapping_cache.selected_sensor == nullptr
                                                 ? false
                                                 : sensor == *new_mapping_cache.selected_sensor;

                if (ImGui::Selectable(sensor.get_imgui_name(), is_selected))
                    new_mapping_cache.selected_sensor = &sensor;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::EndTable();
    }
}

void ViewportPanel::draw_existing_channel_mapping() const noexcept
{
    if (ImGui::BeginTable("##ExistingChannelMapping", 2, table_flags)) {
        ImGui::TableSetupColumn("Signal", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Sensor", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        // Display existing associations.
        for (const auto& [signal, sensor] : active_project->observe_associations()) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            ImGui::Text("%s", signal.get_imgui_name());
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            ImGui::Text("%s", sensor.get_imgui_name());
        }

        ImGui::EndTable();
    }
}

const Signal* ViewportPanel::get_downsampled_signal(
        const Signal& signal
)
{
    auto downsampled_it = downsample_cache.find(signal.get_id());
    bool success;

    if (downsampled_it == downsample_cache.end()) {

        // This is a bit cryptic - https://stackoverflow.com/a/27960637.
        auto [added_it, was_added] = downsample_cache.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(signal.get_id()),
                std::forward_as_tuple(
                        signal,
                        default_downsample_factor,
                        std::format(
                                "{} ({}x LTTB Preview)",
                                signal.get_name(),
                                static_cast<int>(default_downsample_factor)
                        )
                )
        );

        downsampled_it = added_it;
        success = was_added;

        if (success) {
            const auto local_min = added_it->second.begin()->time;
            const auto local_max = std::prev(added_it->second.end())->time;

            if (local_min < waveform_bounding_cache.X.Min)
                waveform_bounding_cache.X.Min = local_min;

            if (local_max > waveform_bounding_cache.X.Max)
                waveform_bounding_cache.X.Max = local_max;
        }
    } else
        success = true;

    return success ? &downsampled_it->second : nullptr;
}

} // namespace WebCFD

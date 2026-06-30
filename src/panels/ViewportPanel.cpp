/**
 * @file
 * @brief WebCFD main viewport panel implementation
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#include "ViewportPanel.hpp"

#include <format>

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
    error_modal.draw();

    if (ImGui::Begin(panel_name.c_str(), nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar)) {
        if (ImGui::BeginTabBar("##ViewportTabBar")) {
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

    ImGui::End();
}

void ViewportPanel::draw_signal_waveforms() const noexcept
{
    if (!ImGui::BeginTabItem("Signal Waveforms"))
        return;

    if (!active_project->get_signal_count())
        ImGui::Text("No signals are loaded.");
    else if (ImPlot::BeginAlignedPlots("WaveformAlignedGroup")) {
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
    }

    ImGui::EndTabItem();
}

void ViewportPanel::draw_sensor_geometry() const noexcept
{
    if (!ImGui::BeginTabItem("Sensor Geometry"))
        return;

    if (!active_project->get_sensors_count())
        ImGui::Text("No sensors are loaded.");
    else if (ImPlot3D::BeginPlot("Sensor Positions")) {
        ImPlot3D::PlotScatterG(
                "",
                Project::get_sensor_point,
                active_project,
                static_cast<int>(active_project->get_sensors_count())
        );
        ImPlot3D::EndPlot();
    }

    ImGui::EndTabItem();
}

void ViewportPanel::draw_channel_mappings() const noexcept
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

void ViewportPanel::draw_new_channel_mapping() const noexcept
{
    if (ImGui::BeginTable("##NewChannelMapping", 2, table_flags)) {
        ImGui::TableSetupColumn("Signal");
        ImGui::TableSetupColumn("Sensor");
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // Prompt for the associated signal.
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
        ImGui::TableSetupColumn("Signal");
        ImGui::TableSetupColumn("Sensor");
        ImGui::TableHeadersRow();

        // Display existing associations.
        for (const auto& [signal, sensor] : active_project->observe_associations()) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", signal.get_imgui_name());
            ImGui::TableNextColumn();
            ImGui::Text("%s", sensor.get_imgui_name());
        }

        ImGui::EndTable();
    }
}

const Signal* ViewportPanel::get_downsampled_signal(
        const Signal& signal
) const
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

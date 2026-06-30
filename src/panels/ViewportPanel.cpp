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

    if (!active_project->are_signals_stored())
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

    if (!active_project->are_sensors_stored())
        ImGui::Text("No sensors are loaded.");
    else if (ImPlot3D::BeginPlot("Sensor Positions")) {
        ImPlot3D::PlotScatterG("", [](const int idx, void * data) -> ImPlot3DPoint {

        });
        ImPlot3D::EndPlot();
    }

    ImGui::EndTabItem();
}

void ViewportPanel::draw_channel_mappings() const noexcept
{
    if (!ImGui::BeginTabItem("Channel Mapping"))
        return;

    if (!active_project->are_associations_defined())
        ImGui::Text("No associations are defined.");
    else if (ImGui::BeginTable("ChannelMappingTable", 3, ImGuiTableFlags_Borders)) {
        ImGui::TableNextColumn();
        ImGui::Text("Signal");
        ImGui::TableNextColumn();
        ImGui::Text("Sensor");
        ImGui::TableNextColumn();
        ImGui::Text("Mapping Status");

        for (const auto& [signal, sensor] : active_project->observe_associations()) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", signal.get_imgui_name());
            ImGui::TableNextColumn();
            ImGui::Text("%s", sensor.get_imgui_name());
            ImGui::TableNextColumn();
            ImGui::Text("OK");
        }

        ImGui::EndTable();
    }

    ImGui::EndTabItem();
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

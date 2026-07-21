/**
 * @file
 * @brief EchoMap sensor geometry panel implementation
 * @author Oliver Dixon
 * @date 2026-07-07
 */

#include "SensorGeometryPanel.hpp"

#include "../EchoMap.hpp"
#include "../objects/Project.hpp"
#include "../objects/Sensor.hpp"
#include "../utility/Logger.hpp"

namespace echomap
{

SensorGeometryPanel::SensorGeometryPanel(
        WorkerResultDespatcher& despatcher,
        EchoMap* app,
        const Project* const initial_project
) :
    active_project(initial_project),
    app(app)
{
    connections.emplace_back(despatcher.load_project_finished_channel.observe([this](const LoadProjectResult& result) {
        active_project = result.observe_project();
        sensor_colours.clear();
    }));
}

const char* SensorGeometryPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void SensorGeometryPanel::draw() noexcept
{
    if (ImGui::Begin(panel_name.c_str())) {
        if (active_project == nullptr)
            ImGui::Text("No project is loaded.");
        else if (active_project->get_sensors_count() == 0u)
            ImGui::Text("No sensors are loaded.");
        else {
            recache_sensor_colours();
            draw_geometry_summary();
            draw_geometry_plot();
        }
    }

    ImGui::End();
}

void SensorGeometryPanel::recache_sensor_colours() noexcept
{
    if (active_project->get_sensors_count() != sensor_colours.size()) {
        // Ensure that we maintain the correct number of colours for the current number of sensors.
        try {
            sensor_colours.resize(active_project->get_sensors_count());
            for (auto [src, dst] : std::views::zip(active_project->observe_sensors(), sensor_colours))
                dst = IM_COL32(
                        static_cast<int>(src.colour.r * 255.0f),
                        static_cast<int>(src.colour.g * 255.0f),
                        static_cast<int>(src.colour.b * 255.0f),
                        static_cast<int>(src.colour.a * 255.0f)
                );
        } catch (const std::exception& exception) {
            LOG_F_ERROR("Could not store the sensor colouring: {}", exception.what());
            plotting_spec_3d.MarkerFillColors = nullptr;
            plotting_spec_3d.MarkerLineColors = nullptr;
            plotting_spec_3d.MarkerFillColor = ImVec4(255.0f, 255.0f, 255.0f, 255.0f);
            plotting_spec_3d.MarkerLineColor = IMPLOT_AUTO_COL;
            return;
        }

        // ... and update the plotting specification in case the resize invalidated pointers.
        plotting_spec_3d.MarkerFillColors = &*sensor_colours.begin();
        plotting_spec_3d.MarkerLineColors = plotting_spec_3d.MarkerFillColors;
    }
}

void SensorGeometryPanel::draw_geometry_summary() noexcept
{
    ImGui::SeparatorText("Geometry Summary");
    if (ImGui::BeginTable("##GeometrySummary", 5, table_flags)) {
        ImGui::TableSetupColumn("##SensorColourColumn", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Sensor", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("X Position", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Y Position", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Z Position", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        std::size_t row_idx = 0;

        for (const auto& sensor : active_project->observe_sensors()) {
            ImGui::PushID(static_cast<int>(sensor.get_id()));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            const ImVec4 colour = ImGui::ColorConvertU32ToFloat4(sensor_colours[row_idx]);
            std::array<float, 4> new_colour = {colour.x, colour.y, colour.z, colour.w};
            if (ImGui::ColorEdit4("##colour", new_colour.data(), ImGuiColorEditFlags_NoInputs)) {
                app->notify(ModifySensorColourNotification(
                        active_project->get_id(),
                        sensor.get_id(),
                        {
                                .r = new_colour[0],
                                .g = new_colour[1],
                                .b = new_colour[2],
                                .a = new_colour[3],
                        }
                ));

                sensor_colours[row_idx] = IM_COL32(
                        static_cast<int>(new_colour[0] * 255.0f),
                        static_cast<int>(new_colour[1] * 255.0f),
                        static_cast<int>(new_colour[2] * 255.0f),
                        static_cast<int>(new_colour[3] * 255.0f)
                );
            }

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            ImGui::TextUnformatted(sensor.get_imgui_name());

            Position new_position = sensor.position;
            bool position_changed = false;

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            position_changed = ImGui::InputFloat("##x", &new_position.x, 0.01f, 1.0f);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            position_changed = ImGui::InputFloat("##y", &new_position.y, 0.01f, 1.0f) || position_changed;

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            position_changed = ImGui::InputFloat("##z", &new_position.z, 0.01f, 1.0f) || position_changed;

            ImGui::PopID();
            ++row_idx;

            if (position_changed)
                app->notify(ModifySensorPositionNotification(active_project->get_id(), sensor.get_id(), new_position));
        }

        ImGui::EndTable();
    }
}

void SensorGeometryPanel::draw_geometry_plot() const noexcept
{
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
}

} // namespace echomap

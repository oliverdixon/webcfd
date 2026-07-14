/**
 * @file
 * @brief EchoMap sensor geometry panel implementation
 * @author Oliver Dixon
 * @date 2026-07-07
 */

#include "SensorGeometryPanel.hpp"

#include "../EchoMap.hpp"
#include "../Logger.hpp"
#include "../objects/Project.hpp"

namespace echomap
{

SensorGeometryPanel::SensorGeometryPanel(
        EchoMap& app,
        const Project* const initial_project
) :
    active_project(initial_project),
    app(app)
{
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
        else if (!active_project->get_sensors_count())
            ImGui::Text("No sensors are loaded.");
        else {
            recache_sensor_colours();
            draw_geometry_summary();
            draw_geometry_plot();
        }
    }

    ImGui::End();
}

void SensorGeometryPanel::set_active_project(
        const Project* new_active_project
) noexcept
{
    active_project = new_active_project;
    sensor_colours.clear();
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
        ImGui::TableSetupColumn("##SensorColourColumn", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Sensor", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("X Position", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Y Position", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Z Position", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        std::size_t row_idx = 0;

        for (auto& sensor : active_project->observe_sensors()) {
            ImGui::PushID(static_cast<int>(sensor.get_id()));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            const ImVec4 colour = ImGui::ColorConvertU32ToFloat4(sensor_colours[row_idx]);
            float new_colour[4] = {colour.x, colour.y, colour.z, colour.w};
            if (ImGui::ColorEdit4("##colour", new_colour, ImGuiColorEditFlags_NoInputs)) {
                app.submit_lightweight_task(ModifySensorColourTask(sensor.get_id(), {
                    new_colour[0], new_colour[1], new_colour[2], new_colour[3]
                }));

                sensor_colours[row_idx] = IM_COL32(
                        static_cast<int>(new_colour[0] * 255.0f),
                        static_cast<int>(new_colour[1] * 255.0f),
                        static_cast<int>(new_colour[2] * 255.0f),
                        static_cast<int>(new_colour[3] * 255.0f)
                );
            }

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            ImGui::Text("%s", sensor.get_imgui_name());

            Sensor::Position new_position = sensor.position;
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
                app.submit_lightweight_task(ModifySensorPositionTask(sensor.get_id(), std::move(new_position)));
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

/**
 * @file
 * @brief WebCFD sensor array position view specification
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#include "ArrayViewPanel.hpp"

#include "../audio/Sensor.hpp"

namespace WebCFD
{

ArrayViewPanel::ArrayViewPanel(
        std::string panel_name,
        const std::vector<Sensor>& sensors
) :
    panel_name(std::move(panel_name)),
    sensors(sensors)
{
    plotting_spec.Stride = sizeof(Sensor);
}

const char* ArrayViewPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void ArrayViewPanel::draw() noexcept
{
    ImGui::Begin(panel_name.c_str(), nullptr);

    if (ImPlot3D::BeginPlot("Sensor Array")) {
        ImPlot3D::PlotScatter(
                "",
                &sensors.begin()->position.x,
                &sensors.begin()->position.y,
                &sensors.begin()->position.z,
                static_cast<int>(sensors.size()),
                plotting_spec
        );

        ImPlot3D::EndPlot();
    }

    ImGui::End();
}

} // namespace WebCFD

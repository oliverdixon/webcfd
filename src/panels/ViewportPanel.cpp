/**
 * @file
 * @brief WebCFD main viewport panel implementation
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#include <imgui.h>
#include <implot.h>

#include "ViewportPanel.hpp"

#include "../audio/WAVData.hpp"

namespace WebCFD
{

ViewportPanel::ViewportPanel()
{
    plotting_spec.Stride = sizeof(AudioChannel::Sample);
}

const char* ViewportPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void ViewportPanel::draw() noexcept
{
    if (ImGui::Begin(panel_name.c_str(), nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar)) {
        if (ImGui::BeginTabBar("##ViewportTabBar")) {
            if (ImGui::BeginTabItem("Signal Waveforms")) {
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Sensor Geometry")) {
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Channel Mapping")) {
                if (ImGui::BeginTable("ChannelMappingTable", 3, ImGuiTableFlags_Borders)) {
                    ImGui::TableNextColumn();
                    ImGui::Text("Signal");
                    ImGui::TableNextColumn();
                    ImGui::Text("Sensor");
                    ImGui::TableNextColumn();
                    ImGui::Text("Mapping Status");

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Localisation")) {
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }

    ImGui::End();
}

} // namespace WebCFD

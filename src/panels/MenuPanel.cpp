//
// Created by owd on 25/06/2026.
//

#include "MenuPanel.hpp"

#include <imgui.h>

namespace WebCFD
{

void MenuPanel::draw() noexcept
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::MenuItem("New Project");
            if (ImGui::BeginMenu("Open Project")) {
                ImGui::MenuItem("... from filesystem");
                if (ImGui::BeginMenu("... from example repository")) {
                    ImGui::MenuItem("Example 1", nullptr, false, false);
                    ImGui::MenuItem("Example 2", nullptr, false, false);
                    ImGui::MenuItem("Example 3", nullptr, false, false);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::MenuItem("Export Project", nullptr, false, false);

            ImGui::Separator();
            ImGui::MenuItem("Import Signal", nullptr, false, false);
            ImGui::MenuItem("Import Sensor Metadata", nullptr, false, false);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Control")) {
            ImGui::MenuItem("Start localisation", nullptr, false, false);
            ImGui::MenuItem("Stop localisation", nullptr, false, false);

            ImGui::Separator();
            ImGui::MenuItem("Configure Algorithm Parameters");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            ImGui::MenuItem("Application Logs");
            ImGui::MenuItem("Platform Information");
            ImGui::MenuItem("About");
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

const char* MenuPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

} // namespace WebCFD

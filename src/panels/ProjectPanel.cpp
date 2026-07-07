//
// Created by owd on 25/06/2026.
//

#include "ProjectPanel.hpp"

#include <imgui.h>

namespace WebCFD
{

ProjectPanel::ProjectPanel(
        Project * const initial_project
) :
    active_project(initial_project)
{
}

void ProjectPanel::draw() noexcept
{
    constexpr ImGuiTreeNodeFlags default_flags = ImGuiTreeNodeFlags_DefaultOpen;

    if (ImGui::Begin(panel_name.c_str())) {
        if (active_project == nullptr)
            ImGui::SeparatorText("No active project.");
        else {
            ImGui::SeparatorText(active_project->get_imgui_name());
            if (ImGui::CollapsingHeader("Signals", default_flags))
                for (const auto& signal : active_project->observe_signals())
                    ImGui::Text("%s", signal.get_imgui_name());

            if (ImGui::CollapsingHeader("Sensors", default_flags))
                for (const auto& sensor : active_project->observe_sensors())
                    ImGui::Text("%s", sensor.get_imgui_name());

            if (ImGui::CollapsingHeader("Results", default_flags)) {
                // TODO...
            }
        }
    }

    ImGui::End();
}

const char* ProjectPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void ProjectPanel::set_active_project(
        Project* const new_active_project
) noexcept
{
    active_project = new_active_project;
}

} // namespace WebCFD

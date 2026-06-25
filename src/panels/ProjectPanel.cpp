//
// Created by owd on 25/06/2026.
//

#include "ProjectPanel.hpp"

#include <imgui.h>

namespace WebCFD
{

void ProjectPanel::draw() noexcept
{
    constexpr ImGuiTreeNodeFlags default_flags = ImGuiTreeNodeFlags_DefaultOpen;

    if (ImGui::Begin(panel_name.c_str())) {
        if (ImGui::CollapsingHeader("Signals", default_flags)) {

        }

        if (ImGui::CollapsingHeader("Sensors", default_flags)) {

        }

        if (ImGui::CollapsingHeader("Results", default_flags)) {

        }
    }

    ImGui::End();
}

const char* ProjectPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

} // namespace WebCFD

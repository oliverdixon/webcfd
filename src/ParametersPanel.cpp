//
// Created by owd on 04/06/2026.
//

#include "ParametersPanel.hpp"

#include <imgui.h>

namespace WebCFD
{

ParametersPanel::ParametersPanel(
        SimulationParameters& parameters,
        std::function<void()> invalidate_layout_callback
) :
    parameters(parameters),
    invalidate_layout_callback(std::move(invalidate_layout_callback))
{
    parameters = default_parameters;
}

const char* ParametersPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void ParametersPanel::draw()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar;

    if (requires_repositioning || force_repositioning) {
        const ImGuiViewport * const viewport = ImGui::GetMainViewport();
        const ImVec2 center{
            viewport->WorkPos.x + viewport->WorkSize.x / 2.0f,
            viewport->WorkPos.y + viewport->WorkSize.y / 2.0f,
        };

        ImGui::SetNextWindowPos(center, force_repositioning ? ImGuiCond_Always : ImGuiCond_FirstUseEver,
            ImVec2{.5f, .5f});
        flags |= ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize;

        requires_repositioning = false;
    }

    ImGui::Begin(panel_name.c_str(), nullptr, flags);

    ImGui::PushItemWidth(256.0f);

    ImGui::SeparatorText("Animation Settings");
    ImGui::SliderFloat("Speed", &parameters.controls.x, 0.0f, 4.0f);
    ImGui::SliderFloat("Intensity", &parameters.controls.y, 0.0f, 3.0f);
    ImGui::SliderFloat("Warp", &parameters.controls.z, 0.0f, 3.0f);
    ImGui::SliderFloat("Scale", &parameters.controls.w, 0.25f, 4.0f);

    ImGui::SeparatorText("Colourway Settings");
    ImGui::ColorEdit3("Colour A", &parameters.colour_a.x);
    ImGui::ColorEdit3("Colour B", &parameters.colour_b.x);
    ImGui::ColorEdit3("Colour C", &parameters.colour_c.x);
    ImGui::ColorEdit3("Colour D", &parameters.colour_d.x);

    ImGui::SeparatorText("Reset Options");
    ImGui::PopItemWidth();

    if (ImGui::Button("Reset Layout")) {
        invalidate_layout_callback();
        force_repositioning = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset Animation Settings"))
        parameters.controls = default_parameters.controls;

    ImGui::SameLine();

    if (ImGui::Button("Reset Colourways")) {
        parameters.colour_a = default_parameters.colour_a;
        parameters.colour_b = default_parameters.colour_b;
        parameters.colour_c = default_parameters.colour_c;
        parameters.colour_d = default_parameters.colour_d;
    }

    ImGui::End();
}

} // namespace WebCFD

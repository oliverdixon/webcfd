//
// Created by owd on 04/06/2026.
//

#include "ParametersPanel.hpp"

#include <imgui.h>

namespace WebCFD
{

ParametersPanel::ParametersPanel(
        SimulationParameters& parameters
) :
    parameters(parameters)
{
}

void ParametersPanel::draw()
{
    ImGui::Begin("Simulation Parameters");

    ImGui::SliderFloat("Speed", &parameters.controls[0], 0.0f, 4.0f);
    ImGui::SliderFloat("Intensity", &parameters.controls[1], 0.0f, 3.0f);
    ImGui::SliderFloat("Warp", &parameters.controls[2], 0.0f, 3.0f);
    ImGui::SliderFloat("Scale", &parameters.controls[3], 0.25f, 4.0f);

    ImGui::ColorEdit4("Colour A", parameters.colour_a);
    ImGui::ColorEdit4("Colour B", parameters.colour_b);
    ImGui::ColorEdit4("Colour C", parameters.colour_c);
    ImGui::ColorEdit4("Colour D", parameters.colour_d);

    ImGui::End();
}

} // namespace WebCFD

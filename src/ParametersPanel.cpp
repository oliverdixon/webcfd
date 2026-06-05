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

    ImGui::ColorEdit4("Triangle Colour", parameters.colour);

    ImGui::End();
}

} // namespace WebCFD

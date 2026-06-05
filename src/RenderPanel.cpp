//
// Created by owd on 04/06/2026.
//

#include <imgui.h>

#include "RenderPanel.hpp"

namespace WebCFD
{

RenderPanel::RenderPanel(
       const wgpu::Device& device,
       const wgpu::TextureFormat texture_format,
       const std::uint32_t width,
       const std::uint32_t height) :
    renderer(device, texture_format, width, height)
{
}

void RenderPanel::draw()
{
    ImGui::Begin("Simulation Visualisation");

    const ImVec2 available_space = ImGui::GetContentRegionAvail();

    // TODO height blows up when hiding the window?  Produces error on console...
    renderer.resize(
        std::max(1u, static_cast<std::uint32_t>(available_space.x)),
        std::max(1u, static_cast<std::uint32_t>(available_space.y))
    );

    if (const auto texture_view = renderer.get_texture_view())
        ImGui::Image(texture_view.Get(), available_space);

    ImGui::End();
}

void RenderPanel::update_gpu(const wgpu::CommandEncoder &command_encoder)
{
    renderer.render(command_encoder);
}

} // WebCFD

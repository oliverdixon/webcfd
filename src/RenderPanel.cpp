//
// Created by owd on 04/06/2026.
//

#include "RenderPanel.hpp"

#include <imgui.h>

namespace WebCFD
{

RenderPanel::RenderPanel(
        const wgpu::Device& device,
        const std::uint32_t width,
        const std::uint32_t height,
        const SimulationParameters& parameters
) :
    renderer(
            device,
            width,
            height,
            parameters
    )
{
}

void RenderPanel::draw()
{
    ImGui::Begin("Simulation Visualisation");

    const auto available_space = ImGui::GetContentRegionAvail();
    renderer.resize(
            std::max(1u, static_cast<std::uint32_t>(available_space.x)),
            std::max(1u, static_cast<std::uint32_t>(available_space.y))
    );

    if (const auto texture_view = renderer.get_texture_view()) {
        /*
         * ImGui v1.91.4 removed ImTextureRef::ImTextureRef(void *) in favour of a (by default) 64-bit type. This
         * reinterpret_cast is acceptable, providing that pointers, in particular WGPUTextureViewImpl*, is no larger
         * than ImTextureID. We statically verify this.
         */
        static_assert(sizeof(WGPUTextureViewImpl *) <= sizeof(ImTextureID));
        ImGui::Image(reinterpret_cast<ImTextureID>(texture_view.Get()), available_space);
    }

    ImGui::End();
}

void RenderPanel::update_gpu(
        const wgpu::CommandEncoder& command_encoder
)
{
    renderer.render(command_encoder);
}

} // namespace WebCFD

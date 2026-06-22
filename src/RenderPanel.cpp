/**
 * @file
 * @brief WebCFD rendering ImGui panel implementation
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#include "RenderPanel.hpp"

#include <imgui.h>

namespace WebCFD
{

RenderPanel::RenderPanel(
        std::string panel_name,
        const ViewportRenderer::Shader shader,
        wgpu::Device device,
        const std::uint32_t width,
        const std::uint32_t height,
        const SimulationParameters& parameters
) :
    renderer(
            std::move(device),
            width,
            height,
            shader,
            parameters
    ),
    panel_name(std::move(panel_name))
{
}

void RenderPanel::draw()
{
    ImGui::Begin(panel_name.c_str());

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
        static_assert(sizeof(WGPUTextureViewImpl*) <= sizeof(ImTextureID));
        ImGui::Image(reinterpret_cast<ImTextureID>(texture_view.Get()), available_space);
    }

    ImGui::End();
}

const char* RenderPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void RenderPanel::update_gpu(
        const wgpu::CommandEncoder& command_encoder
)
{
    renderer.render(command_encoder);
}

} // namespace WebCFD

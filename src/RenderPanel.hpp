//
// Created by owd on 04/06/2026.
//

#ifndef WEBCFD_RENDERPANEL_HPP
#define WEBCFD_RENDERPANEL_HPP

#include <string>

#include "IPanel.hpp"
#include "ViewportRenderer.hpp"

namespace WebCFD
{

class RenderPanel final : public IPanel
{
public:
    RenderPanel(
            std::string panel_name,
            ViewportRenderer::Shader shader,
            const wgpu::Device& device,
            std::uint32_t width,
            std::uint32_t height,
            const SimulationParameters& parameters
    );

    void draw() override;

    const char* get_imgui_name() const noexcept override;

    void update_gpu(const wgpu::CommandEncoder& command_encoder) override;

private:
    ViewportRenderer renderer;
    std::string panel_name;
};

} // namespace WebCFD

#endif // WEBCFD_RENDERPANEL_HPP

//
// Created by owd on 04/06/2026.
//

#ifndef WEBCFD_RENDERPANEL_HPP
#define WEBCFD_RENDERPANEL_HPP

#include "IPanel.hpp"
#include "ViewportRenderer.hpp"

namespace WebCFD
{

class RenderPanel final : public IPanel
{
public:
    RenderPanel(
            const wgpu::Device& device,
            std::uint32_t width,
            std::uint32_t height,
            const SimulationParameters& parameters
    );

    void draw() override;

    void update_gpu(const wgpu::CommandEncoder& command_encoder) override;

private:
    ViewportRenderer renderer;
};

} // namespace WebCFD

#endif // WEBCFD_RENDERPANEL_HPP

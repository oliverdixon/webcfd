/**
 * @file
 * @brief WebCFD rendering ImGui panel specification
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#ifndef WEBCFD_RENDERPANEL_HPP
#define WEBCFD_RENDERPANEL_HPP

#include <string>

#include "IPanel.hpp"
#include "ViewportRenderer.hpp"

namespace WebCFD
{

/**
 * Defines a titled Dear ImGui panel for displaying image data from the WebGPU context.
 *
 * The RenderPanel owns its own renderer, as a ViewportRenderer (or equivalent). The panel is notified when the GPU is
 * ready to accept more work on the given device through the interface methods. Note that, like the ParametersPanel,
 * RenderPanel instances are created subordinate to the WebCFD instance and contain weak references to its state.
 * Therefore, no RenderPanel should outlive its parent WebCFD.
 */
class RenderPanel final : public IPanel
{
public:
    /**
     * Create a new RenderPanel and rendering context to compute and render the chosen shader.
     *
     * @param panel_name Name of the panel, presented on the GUI as a window title.
     * @param shader The choice of shader to present on the panel.
     * @param device The initialised WebGPU Device to service the rendering context.
     * @param width The initial width of the viewport, in pixels.
     * @param height The initial height of the viewport, in pixels.
     * @param parameters The SimulationParameters for the simulation, to be provided as a uniform buffer to the shader.
     */
    RenderPanel(
            std::string panel_name,
            ViewportRenderer::Shader shader,
            wgpu::Device device,
            std::uint32_t width,
            std::uint32_t height,
            const SimulationParameters& parameters
    );

    void draw() override;

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void update_gpu(const wgpu::CommandEncoder& command_encoder) override;

private:
    ViewportRenderer renderer;
    std::string panel_name;
};

} // namespace WebCFD

#endif // WEBCFD_RENDERPANEL_HPP

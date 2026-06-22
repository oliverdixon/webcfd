/**
 * @file
 * @brief WebCFD viewport renderer specification
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#ifndef WEBCFD_VIEWPORTRENDERER_HPP
#define WEBCFD_VIEWPORTRENDERER_HPP

#include <webgpu/webgpu_cpp.h>

#include "SimulationParameters.hpp"

namespace WebCFD
{

/**
 * Provides a renderer and WebGPU-compatible rendering context for a predefined shader that is parameterisable in real
 *  time.
 */
class ViewportRenderer
{
public:
    /**
     * Pre-defined shader selection
     */
    enum class Shader
    {
        Aurora,
        JuliaBloom,
        NeonVoronoi,
        Vortex
    };

    /**
     * Create a new ViewportRenderer to service the selected shader.
     *
     * @param device The initialised WebGPU device to manage the rendering pipeline and buffers.
     * @param width The initial viewport width, in pixels.
     * @param height The initial viewport height, in pixels.
     * @param shader The shader to use.
     * @param parameters A weak reference to the SimulationParameters to use to parameterise the appearance.
     */
    ViewportRenderer(
            wgpu::Device device,
            std::uint32_t width,
            std::uint32_t height,
            Shader shader,
            const SimulationParameters& parameters
    );

    /**
     * Despatch work to the GPU by the given WebGPU CommandEncoder to render the next shader step.
     *
     * @param command_encoder
     */
    void render(const wgpu::CommandEncoder& command_encoder);

    /**
     * Resize the viewport size.
     *
     * @param new_width New width of the viewport, in pixels.
     * @param new_height New height of the viewport, in pixels.
     */
    void resize(
            std::uint32_t new_width,
            std::uint32_t new_height
    );

    /**
     * Retrieves a reference to the WebGPU TextureView maintained by the ViewportRenderer.
     *
     * @return The WebGPU TextureView to which the image data is copied from the GPU.
     */
    [[nodiscard]] wgpu::TextureView get_texture_view() const noexcept;

private:
    // TODO missing doxygen...

    void recreate_texture();
    void create_parameter_buffer();
    void create_pipeline();

    static constexpr auto texture_format = wgpu::TextureFormat::RGBA8Unorm;

    std::uint32_t width;
    std::uint32_t height;

    ShaderVec4 viewport;
    const SimulationParameters& parameters;

    wgpu::Buffer uniform_buffer;
    wgpu::BindGroup bind_group;
    wgpu::StringView shader_code;

    wgpu::Device device;
    wgpu::RenderPipeline pipeline;
    wgpu::Texture texture;
    wgpu::TextureView texture_view;
};

} // namespace WebCFD

#endif // WEBCFD_VIEWPORTRENDERER_HPP

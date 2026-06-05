//
// Created by owd on 04/06/2026.
//

#ifndef WEBCFD_VIEWPORTRENDERER_HPP
#define WEBCFD_VIEWPORTRENDERER_HPP

#include <webgpu/webgpu_cpp.h>

#include "SimulationParameters.hpp"

namespace WebCFD
{

class ViewportRenderer
{
public:
    ViewportRenderer(
            const wgpu::Device& device,
            std::uint32_t width,
            std::uint32_t height,
            const SimulationParameters& parameters
    );

    void render(const wgpu::CommandEncoder& command_encoder) const;

    void resize(
            std::uint32_t new_width,
            std::uint32_t new_height
    );

    [[nodiscard]] wgpu::TextureView get_texture_view() const noexcept;

private:
    void recreate_texture();
    void create_parameter_buffer();
    void create_pipeline();

    static constexpr auto texture_format = wgpu::TextureFormat::RGBA8Unorm;

    std::uint32_t width;
    std::uint32_t height;
    const SimulationParameters& parameters;
    wgpu::Buffer uniform_buffer;
    wgpu::BindGroup bind_group;

    wgpu::Device device;
    wgpu::RenderPipeline pipeline;
    wgpu::Texture texture;
    wgpu::TextureView texture_view;
};

} // namespace WebCFD

#endif // WEBCFD_VIEWPORTRENDERER_HPP

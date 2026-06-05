//
// Created by owd on 04/06/2026.
//

#ifndef WEBCFD_VIEWPORTRENDERER_HPP
#define WEBCFD_VIEWPORTRENDERER_HPP

#include <webgpu/webgpu_cpp.h>

namespace WebCFD
{

class ViewportRenderer
{
public:
    ViewportRenderer(const wgpu::Device& device, wgpu::TextureFormat texture_format, std::uint32_t width,
        std::uint32_t height);

    void render(const wgpu::CommandEncoder &command_encoder) const;

    void resize(std::uint32_t new_width, std::uint32_t new_height);

    [[nodiscard]] wgpu::TextureView get_texture_view() const noexcept;

private:
    void install_shader();
    void recreate_texture();

    std::uint32_t width;
    std::uint32_t height;

    wgpu::Device device;
    wgpu::TextureFormat texture_format;
    wgpu::RenderPipeline pipeline;
    wgpu::Texture texture;
    wgpu::TextureView texture_view;
};

} // WebCFD

#endif //WEBCFD_VIEWPORTRENDERER_HPP

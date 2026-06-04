//
// Created by owd on 04/06/2026.
//

#ifndef WEBCFD_WEBCFD_HPP
#define WEBCFD_WEBCFD_HPP

#include <GLFW/glfw3.h>
#include <webgpu/webgpu_cpp.h>

namespace WebCFD
{

class WebCFD
{
public:
    WebCFD();

    ~WebCFD();

    void start();

private:
    static constexpr std::uint32_t default_width = 512;
    static constexpr std::uint32_t default_height = 512;
    static constexpr auto default_timeout = std::numeric_limits<std::uint64_t>::max();

    static GLFWwindow *create_window();
    wgpu::Future request_adapter();
    wgpu::Future request_device();
    void configure_surface();
    void configure_sample_shader();

    void render() const;

#ifdef __EMSCRIPTEN__
    // ReSharper disable once CppParameterMayBeConstPtrOrRef - Function signature enforced by Emscripten API.
    static void render_shim(void * const user_data)
    {
        const auto* instance = static_cast<const WebCFD*>(user_data);
        instance->render();
    }
#endif

    wgpu::Instance instance;
    wgpu::Adapter adapter;
    wgpu::Device device;
    wgpu::Surface surface;
    wgpu::TextureFormat format;
    wgpu::RenderPipeline pipeline;
    GLFWwindow * window;
};

} // WebGPU

#endif // WEBCFD_WEBCFD_HPP

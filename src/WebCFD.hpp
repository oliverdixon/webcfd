//
// Created by owd on 04/06/2026.
//

#ifndef WEBCFD_WEBCFD_HPP
#define WEBCFD_WEBCFD_HPP

#include <GLFW/glfw3.h>
#include <webgpu/webgpu_cpp.h>

#include "IPanel.hpp"

namespace WebCFD
{

class WebCFD
{
public:
    WebCFD();

    ~WebCFD();

private:
    static constexpr auto operation_timeout = std::numeric_limits<std::uint64_t>::max();

    std::uint32_t viewport_width = 512;
    std::uint32_t viewport_height = 512;

    static GLFWwindow* create_window(
            int width,
            int height
    );

    static void configure_surface(
            const wgpu::Surface& surface,
            const wgpu::Device& device,
            const wgpu::SurfaceCapabilities& capabilities,
            std::uint32_t viewport_width,
            std::uint32_t viewport_height
    );

    wgpu::Future request_adapter();
    wgpu::Future request_device();

    void run_event_loop();
    void render();
    void setup_gui();
    bool handle_window_resize();

#ifdef __EMSCRIPTEN__

    // ReSharper disable once CppParameterMayBeConstPtrOrRef - Function signature enforced by Emscripten API.
    /**
     * Invokes WebCFD::render from a static context given an untyped mutable pointer to the WebCFD object instance.
     *
     * @note This function provided when targeting Emscripten only. It is intended to be used as a function callback
     *  from the Emscripten C API.
     *
     * @param webcfd_instance The WebCFD application instance on which to invoke WebCFD::render.
     */
    static void render_shim(
            void* const webcfd_instance
    )
    {
        auto* instance = static_cast<WebCFD*>(webcfd_instance);
        instance->render();
    }
#endif

    wgpu::Instance instance;
    wgpu::Adapter adapter;
    wgpu::Device device;
    wgpu::Surface surface;
    wgpu::SurfaceCapabilities surface_capabilities;
    GLFWwindow* window = nullptr;

    std::vector<std::unique_ptr<IPanel>> panels;
};

} // namespace WebCFD

#endif // WEBCFD_WEBCFD_HPP

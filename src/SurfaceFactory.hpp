/**
 * @file
 * @brief EchoMap WebGPU-GLFW Surface factory specification
 * @author Oliver Dixon
 * @date 2026-07-05
 */

#ifndef ECHOMAP_SURFACEFACTORY_HPP
#define ECHOMAP_SURFACEFACTORY_HPP

#include <webgpu/webgpu_cpp.h>

struct GLFWwindow;

namespace echomap
{

/**
 * Provides static helpers to create and bind a WebGPU Surface to a GLFW window across Wasm and non-Wasm platforms.
 */
class SurfaceFactory
{
public:
    /**
     * Creates and binds a Surface to the given GLFW window.
     *
     * @param instance The WebGPU Instance in which the Surface should be created.
     * @param window The GLFW window pointer to the target window.
     * @return A Surface registered to the Instance and bound to the GLFWwindow.
     *
     * @throws std::runtime_error A Surface could not be created and bound for the given Window.
     */
    static wgpu::Surface create_surface(
            const wgpu::Instance& instance,
            GLFWwindow* window
    );

private:
    // ReSharper disable once CppDoxygenSyntaxError
    /**
     * Produce an owning container of the WebGPU Surface descriptor for the given GLFW window.
     *
     * <p>
     *  Supported platforms:
     *  <ul>
     *      <li>Emscripten</li>
     *      <li>Wayland</li>
     *      <li>X11</li>
     *      <li>Win32</li>
     *      <li>Cocoa/Apple</li>
     *  </ul>
     * </p>
     *
     * @param window The GLFW window to bind to the WebGPU surface.
     * @return An owning container detaining the chain. The next chained element is the bound Surface. If a suitable
     * Surface could not be bound, <code>nullptr</code> is returned.
     */
    static std::unique_ptr<wgpu::ChainedStruct> get_surface_descriptor(GLFWwindow* window);
};

} // namespace echomap

#endif // ECHOMAP_SURFACEFACTORY_HPP

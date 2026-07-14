/**
 * @file
 * @brief EchoMap class specification
 * @author Oliver Dixon
 * @date 2026-05-05
 */

#ifndef ECHOMAP_ECHOMAP_HPP
#define ECHOMAP_ECHOMAP_HPP

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <webgpu/webgpu_cpp.h>

#include "tasks/IResultHandler.hpp"
#include "tasks/Worker.hpp"
#include "tasks/lightweight/AddChannelMappingTask.hpp"
#include "tasks/lightweight/ModifySensorColourTask.hpp"
#include "tasks/lightweight/ModifySensorPositionTask.hpp"

namespace echomap
{

class IPanel;
class Project;

/**
 * The EchoMap maintains state for the application including WebGPU and Dear ImGui context, encapsulating initialisation,
 * game loop, interaction, and clean-up.
 */
class EchoMap : IResultHandler
{
public:
    using LightweightTask = std::variant<AddChannelMappingTask, ModifySensorColourTask, ModifySensorPositionTask>;

    /**
     * Initialise a EchoMap application instance.
     *
     * Initialisation is a computationally substantial task. Context from all managed frameworks must be initialised
     * (GLFW, WebGPU/Dawn, and Dear ImGui) and their components registered. Once the constructor has completed, the game
     * loop can begin with @ref run_event_loop.
     *
     * @throws ConfigurationError Some part of initialisation, described in the exception message, did not succeed.
     */
    EchoMap();

    /**
     * Runs the event loop to manage and propagate interaction with the EchoMap application.
     *
     * This function returns only once GLFW indicates that the window should close. Following closure, the event loop
     * could be re-run, or the application could clean up by calling the destructor.
     */
    void run_event_loop();

    /**
     * Clean up all persistent state registered by the application instance.
     */
    ~EchoMap() noexcept override;

    /**
     * Indicate to the wave form controllers that a new file has been selected.
     *
     * @param path Path of the new wave file on the file system.
     */
    void update_wav_file(const char* path);

    [[nodiscard]] std::unique_ptr<Project> take_project(bool update_ui) noexcept;
    void put_project(std::unique_ptr<Project> new_project) noexcept;

    /**
     * Submit a new lightweight task to the application queue.
     *
     * @param task The trivial task to schedule.
     */
    void submit_lightweight_task(LightweightTask task);

    void handle(LoadProjectResult& result) override;
    void handle(DownsampleResult& result) override;
    void handle(DFTResult& result) override;

private:
    static constexpr auto operation_timeout = std::numeric_limits<std::uint64_t>::max();

    std::uint32_t viewport_width = 1024;
    std::uint32_t viewport_height = 1024;

    /**
     * Create a new GLFW window of the specified dimensions from a static context.
     *
     * @param width Initial width of the window, in pixels.
     * @param height Initial height of the window, in pixels.
     * @return A mutable pointer to the created window, which must be explicitly deleted following use.
     * @throws ConfigurationError A GLFW initialisation step failed.
     */
    static GLFWwindow* create_window(
            int width,
            int height
    );

    /**
     * Configure a WebGPU Surface from a static context given metadata and Adapter capabilities.
     *
     * @note There is no way to check if the given Surface is already configured from the public WebGPU.h API. Moreover,
     *  attempting to Unconfigure an unconfigured Surface will assert. Therefore, callers must ensure that the given
     *  Surface is in an unconfigured state prior to invoking this function, as it cannot sanity-check the state of the
     *  Surface.
     *
     * @param surface The Surface to configure.
     * @param device The WebGPU Device on which the Surface will be displayed.
     * @param capabilities Capabilities of the WebGPU Adapter and Instance.
     * @param viewport_width Initial width of the Surface viewport, in pixels.
     * @param viewport_height Initial height of the Surface viewport, in pixels.
     */
    static void configure_surface(
            const wgpu::Surface& surface,
            const wgpu::Device& device,
            const wgpu::SurfaceCapabilities& capabilities,
            std::uint32_t viewport_width,
            std::uint32_t viewport_height
    ) noexcept;

    /**
     * Produce a WebGPU Future for requesting an Adapter.
     *
     * @return A Future to request an Adapter that is suitable for the Surface member from the WebGPU driver.
     */
    wgpu::Future request_adapter() noexcept;

    /**
     * Produce a WebGPU Future for requesting an accelerator device.
     *
     * @return A Future to request a Device from the WebGPU driver.
     */
    wgpu::Future request_device() noexcept;

    /**
     * Perform a render cycle on the configured Surface and Device.
     *
     * A single render cycle requests all panels to render their state to the Surface, and provides an opportunity to
     * submit any work to the GPU. Events are also received from GLFW and processed as required.
     */
    void render() noexcept;

    /**
     * Create a context for Dear ImGui and ImPlot, and configure the plain GLFW and WebGPU backends.
     *
     * @throws ConfigurationError A Dear ImGui backend could not be initialised.
     */
    void setup_imgui();

    /**
     * Check if the window has been resized compared with the stored dimensions, updating member variables and
     * reconfiguring the WebGPU surface if necessary.
     *
     * @return Is the window visible?
     */
    bool handle_window_resize() noexcept;

    void process_worker_results();

    void update_panel_project() const;

#ifdef __EMSCRIPTEN__

    // ReSharper disable once CppParameterMayBeConstPtrOrRef - Function signature enforced by Emscripten API.
    /**
     * Invokes echomap::render from a static context given an untyped mutable pointer to the EchoMap object instance.
     *
     * @note This function provided when targeting Emscripten only. It is intended to be used as a function callback
     *  from the Emscripten C API.
     *
     * @param echomap_instance The EchoMap application instance on which to invoke echomap::render.
     */
    static void render_shim(
            void* const echomap_instance
    )
    {
        auto* instance = static_cast<EchoMap*>(echomap_instance);
        instance->render();
    }
#endif

    wgpu::Instance instance;
    wgpu::Adapter adapter;
    wgpu::Device device;
    wgpu::Surface surface;
    wgpu::SurfaceCapabilities surface_capabilities;
    GLFWwindow* window = nullptr;

    std::unique_ptr<Project> project;
    std::vector<std::unique_ptr<IPanel>> panels;

    Worker worker;
    std::vector<LightweightTask> lightweight_tasks;

    ImGuiID dockspace_id;
    bool dockspace_configured = false;
};

} // namespace echomap

#endif // ECHOMAP_ECHOMAP_HPP

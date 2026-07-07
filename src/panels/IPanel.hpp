/**
 * @file
 * @brief Panel interface specification
 * @author Oliver Dixon
 * @date 2026-05-05
 */

#ifndef WEBCFD_PANEL_HPP
#define WEBCFD_PANEL_HPP

#include <webgpu/webgpu_cpp.h>

namespace WebCFD
{

class Project;

/**
 * Represents a panel or window on the GUI.
 *
 * Panels may render from a static context, typically through a configured ImGui context, or despatch work to the GPU
 *  through a WebGPU CommandEncoder.
 */
class IPanel
{
public:
    virtual ~IPanel() = default;

    /**
     * Draw the panel to the active rendering context.
     */
    virtual void draw() noexcept = 0;

    /**
     * Retrieves the name of the IPanel for Dear ImGui API functions.
     *
     * @return The human-readable name of the window as a NULL-terminated C string.
     */
    [[nodiscard]] virtual const char* get_imgui_name() const noexcept = 0;

    /**
     * Delegate work to the GPU via the WebGPU platform.
     *
     * @param command_encoder The configured CommandEncoder for the target accelerator.
     */
    virtual void update_gpu(
            const wgpu::CommandEncoder& command_encoder
    )
    {
        std::ignore = command_encoder;
    }

    /**
     * Reset the active Project for a Panel.
     *
     * This functionally intentionally does not provide a default. All panels must acknowledge a new active project,
     * even if implemented as a no-op.
     *
     * @param new_active_project A mutable pointer to the new active Project.
     *
     * @todo Panels should be able to get by with a constant pointer. The application should use an event-driven
     *       architecture, such that panels do not modify the Project directly, but post change requests to a queue
     *       managed by the central WebCFD instance. This would also facilitiate moving to a multi-threaded
     *       architecture, since the render actions would have less data dependency on the Project state.
     */
    virtual void set_active_project(Project* new_active_project) noexcept = 0;
};

} // namespace WebCFD

#endif // WEBCFD_PANEL_HPP

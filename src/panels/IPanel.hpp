/**
 * @file
 * @brief Panel interface specification
 * @author Oliver Dixon
 * @date 2026-05-05
 */

#ifndef ECHOMAP_PANEL_HPP
#define ECHOMAP_PANEL_HPP

#include <webgpu/webgpu_cpp.h>
// ReSharper disable once CppUnusedIncludeDirective - False positive. Required for flags.
#include <implot.h>

#include "../tasks/IResultHandler.hpp"

namespace echomap
{

class Project;

/**
 * Represents a panel or window on the GUI.
 *
 * Panels may render from a static context, typically through a configured ImGui context, or despatch work to the GPU
 * through a WebGPU CommandEncoder.
 *
 * @todo All panels shouldn't necessarily implement IResultHandler. We might want a more "precise" pub-sub model.
 */
class IPanel : public IResultHandler
{
public:
    ~IPanel() override = default;

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
     * @param new_active_project An observing pointer to the new active Project.
     */
    virtual void set_active_project(const Project* new_active_project) noexcept = 0;

protected:
    static constexpr auto table_flags =
            ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
};

} // namespace echomap

#endif // ECHOMAP_PANEL_HPP

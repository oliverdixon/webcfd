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

namespace echomap
{

class Project;

/**
 * Represents a panel or window on the GUI.
 *
 * Panels may render from a static context, typically through a configured ImGui context, or despatch work to the GPU
 * through a WebGPU CommandEncoder.
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
     * Updates the active Project being described by the IPanel.
     *
     * @param new_project An observing pointer to the new active Project.
     */
    virtual void change_active_project(const Project * new_project) = 0;

protected:
    constexpr static ImVec2 button_size{80.0f, 20.0f};

    // NOLINTBEGIN(*-signed-bitwise) - Intended ImGui API usage.
    static constexpr auto table_flags =
            ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    // NOLINTEND(*-signed-bitwise)
};

} // namespace echomap

#endif // ECHOMAP_PANEL_HPP

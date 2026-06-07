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
    virtual void draw() = 0;

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
};

} // namespace WebCFD

#endif // WEBCFD_PANEL_HPP

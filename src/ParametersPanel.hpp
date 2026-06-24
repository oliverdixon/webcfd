/**
 * @file
 * @brief WebCFD parameters ImGui panel specification
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#ifndef WEBCFD_PARAMETERSPANEL_HPP
#define WEBCFD_PARAMETERSPANEL_HPP

#include <implot.h>

#include <functional>
#include <string>

#include "IPanel.hpp"
#include "WAVData.hpp"

namespace WebCFD
{

/**
 * Defines a Dear ImGui panel to dynamically control simulation parameters.
 *
 * This class contains weak references to callbacks and the mutable SimulationParameters structure persisted by a WebCFD
 * instance. Therefore, the lifetime of the ParametersPanel should not outlive the parent WebCFD instance.
 */
class ParametersPanel final : public IPanel
{
public:
    /**
     * Create a new ParametersPanel to control the given SimulationParameters.
     *
     * @param invalidate_layout_callback The callback to invalidate the layout of the parent window.
     */
    explicit ParametersPanel(
            std::function<void()> invalidate_layout_callback
    );

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() override;

private:
    const std::string panel_name = "Simulation Parameters";

    bool requires_repositioning = true;
    bool force_repositioning = false;

    WAVData wav_data{"../audio/stereo.wav"};
    WAVData downsampled{wav_data, 50.0f};

    ImPlotSpec plotting_spec;
    std::function<void()> invalidate_layout_callback;
};

} // namespace WebCFD

#endif // WEBCFD_PARAMETERSPANEL_HPP

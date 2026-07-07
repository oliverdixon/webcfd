/**
 * @file
 * @brief WebCFD signal waveform preview panel specification
 * @author Oliver Dixon
 * @date 2026-07-07
 */

#ifndef WEBCFD_SIGNALWAVEFORMPANEL_HPP
#define WEBCFD_SIGNALWAVEFORMPANEL_HPP

#include <implot.h>

#include "../objects/Signal.hpp"
#include "IPanel.hpp"

namespace WebCFD
{

/**
 * The SignalWaveformPanel provides a preview of downsampled time-series waveforms of each Signal in the active Project.
 *
 * Additionally, the panel owns the downsampled time series in a cache used exclusively for visualisation.
 */
class SignalWaveformPanel : public IPanel
{
public:
    explicit SignalWaveformPanel(Project* initial_project = nullptr);

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() noexcept override;

    void set_active_project(Project* new_active_project) noexcept override;

private:
    static constexpr float default_downsample_factor = 50.0f;

    /**
     * Retrieves a downsampled Signal from the cache. If the downsampled variant is not present, it is computed and
     * cached.
     *
     * @param signal The original Signal to downsample.
     * @return A stable observing pointer to the downsampled Signal, or null if it could not be produced.
     *
     * @todo It would be nice if this could run in the background, and not hold up the render thread. At least, the
     *       cache miss (downsampling algorithm) case shouldn't be done in the render thread.
     */
    const Signal* get_downsampled_signal(const Signal& signal);

    /**
     * The duplicated Signal objects, downsampled for visualisation.
     *
     * It is typically undesirable for the panels to own any non-trivial state. In this case, however, the data is
     * generated exclusively for the purposes of visualisation on this panel, and the elements' construction and
     * destruction is at the whim of the renderer such that downsampled time series are only created ad-hoc when they
     * need to be visualised. Therefore, the ViewportPanel seems to be the natural owner.
     */
    std::unordered_map<Signal::id_type, Signal> downsample_cache;

    /**
     * The maximum bounding box of a LTTB-downsampled wave form plot.
     *
     * The Y axis (amplitude) is fixed, since our PCM-normalised values, given as a constraint from the Signal samples,
     * are within the range [-1, 1]. The X range is variable and should be updated as new Signal objects are received.
     */
    ImPlotRect waveform_bounding_cache{
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::lowest(),
            -1.0,
            1.0
    };

    const std::string panel_name = "Signal Waveform Preview";

    ImPlotSpec plotting_spec_2d;
    Project* active_project = nullptr;
};

} // namespace WebCFD

#endif // WEBCFD_SIGNALWAVEFORMPANEL_HPP

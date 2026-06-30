/**
 * @file
 * @brief WebCFD main viewport panel specification
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#ifndef WEBCFD_VIEWPORTPANEL_HPP
#define WEBCFD_VIEWPORTPANEL_HPP

#include <implot.h>
#include <implot3d.h>

#include <string>

#include "../Project.hpp"
#include "IPanel.hpp"

namespace WebCFD
{

class ViewportPanel final : public IPanel
{
public:
    explicit ViewportPanel(Project* initial_project = nullptr);

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() noexcept override;

private:
    void draw_signal_waveforms() const noexcept;
    void draw_sensor_geometry() const noexcept;
    void draw_channel_mappings() const noexcept;

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
    const Signal* get_downsampled_signal(const Signal& signal) const;

    /**
     * The duplicated Signal objects, downsampled for visualisation.
     *
     * It is typically undesirable for the panels to own any non-trivial state. In this case, however, the data is
     * generated exclusively for the purposes of visualisation on this panel, and the elements' construction and
     * destruction is at the whim of the renderer such that downsampled time series are only created ad-hoc when they
     * need to be visualised. Therefore, the ViewportPanel seems to be the natural owner.
     *
     * @note This field is declared <code>mutable</code> since it is a cache modifiable in the rendering routine.
     */
    mutable std::unordered_map<Signal::id_type, Signal> downsample_cache;

    /**
     * The maximum bounding box of a LTTB-downsampled wave form plot.
     *
     * The Y axis (amplitude) is fixed, since our PCM-normalised values, given as a constraint from the Signal samples,
     * are within the range [-1, 1]. The X range is variable and should be updated as new Signal objects are received.
     *
     * @note This field is declared <code>mutable</code> since it is a cache modifiable in the rendering routine.
     */
    mutable ImPlotRect waveform_bounding_cache{
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::lowest(),
            -1.0,
            1.0
    };

    const std::string panel_name = "Manager";
    static constexpr float default_downsample_factor = 50.0f;

    ImPlotSpec plotting_spec_2d;
    ImPlot3DSpec plotting_spec_3d;

    Project* active_project = nullptr;
};

} // namespace WebCFD

#endif // WEBCFD_VIEWPORTPANEL_HPP

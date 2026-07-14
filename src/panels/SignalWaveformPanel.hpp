/**
 * @file
 * @brief EchoMap signal waveform preview panel specification
 * @author Oliver Dixon
 * @date 2026-07-07
 */

#ifndef ECHOMAP_SIGNALWAVEFORMPANEL_HPP
#define ECHOMAP_SIGNALWAVEFORMPANEL_HPP

#include <implot.h>

#include "../objects/FrequencySpectrum.hpp"
#include "../objects/Signal.hpp"
#include "IPanel.hpp"

namespace echomap
{

class Worker;

/**
 * The SignalWaveformPanel provides a preview of downsampled time-series waveforms of each Signal in the active Project.
 *
 * Additionally, the panel owns the downsampled time series in a cache used exclusively for visualisation.
 */
class SignalWaveformPanel final : public IPanel
{
public:
    explicit SignalWaveformPanel(Worker& parent_worker, Project* initial_project = nullptr);

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() noexcept override;

    void set_active_project(Project* new_active_project) noexcept override;

    void handle(DownsampleResult& result) override;

private:
    static constexpr float default_downsample_factor = 50.0f;

    struct SignalCallback
    {
        const Signal * signal;
    };

    struct FSCallback
    {
        const FrequencySpectrum* spectrum;
    };

    static ImPlotPoint get_indexed_signal_point(
            int index,
            void* user_data
    ) noexcept;

    static ImPlotPoint get_indexed_frequency_bin(
            int index,
            void* user_data
    ) noexcept;

    void update_bounding_box(const Signal& signal);
    void update_bounding_box(const FrequencySpectrum& spectrum);
    void update_bounding_box();

    /**
     * Retrieves a downsampled Signal from the cache. If the downsampled variant is not present, it is computed and
     * cached.
     *
     * @param signal The original Signal to downsample.
     * @return A stable observing pointer to the downsampled Signal, or null if it could not be produced at this time.
     *
     * @pre The given Signal container must detain a valid Signal object.
     */
    const Signal* get_downsampled_signal(std::shared_ptr<Signal> signal);

    const FrequencySpectrum* get_spectra(std::shared_ptr<Signal> signal);

    /**
     * The duplicated Signal objects, downsampled for visualisation.
     *
     * <p>
     *  This map associates the IDs of the original Signal objects with owning containers of their downsampled
     *  counterparts.
     * </p>
     * <p>
     *  It is typically undesirable for the panels to own any non-trivial state. In this case, however, the data is
     *  generated exclusively for the purposes of visualisation on this panel, and the elements' construction and
     *  destruction is at the whim of the renderer such that downsampled time series are only created ad-hoc when they
     *  need to be visualised. Therefore, the SignalWaveformPanel seems to be the natural owner.
     * </p>
     */
    std::unordered_map<Signal::id_type, std::unique_ptr<Signal>> downsample_cache;

    std::unordered_map<Signal::id_type, std::unique_ptr<FrequencySpectrum>> spectra_cache; // TODO not here...

    /**
     * The maximum bounding box of a LTTB-downsampled wave form plot.
     *
     * The Y axis (amplitude) is fixed, since our PCM-normalised values, given as a constraint from the Signal samples,
     * are within the range [-1, 1]. The X range is variable and should be updated as new Signal objects are received.
     */
    ImPlotRect waveform_bounding_box{
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::lowest(),
            -1.0,
            1.0
    };

    const std::string panel_name = "Signal Waveform Preview";

    ImPlotSpec plotting_spec_2d;
    Worker& parent_worker;
    Project* active_project = nullptr;
};

} // namespace echomap

#endif // ECHOMAP_SIGNALWAVEFORMPANEL_HPP

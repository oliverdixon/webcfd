/**
 * @file
 *
 * SignalDFTPanel specification
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#ifndef ECHOMAP_SIGNALDFTPANEL_HPP
#define ECHOMAP_SIGNALDFTPANEL_HPP

#include "../objects/FrequencySpectrum.hpp"
#include "../objects/Signal.hpp"
#include "IPanel.hpp"

namespace echomap
{

class Worker;

/**
 * Provides an IPanel to display and interact with previews of Signal frequency spectra (i.e., Signal DFTs).
 */
class SignalDFTPanel : public IPanel
{
public:
    explicit SignalDFTPanel(
            Worker& parent_worker,
            Project* initial_project = nullptr
    );

    void draw() noexcept override;

    const char* get_imgui_name() const noexcept override;

    void set_active_project(Project* new_active_project) noexcept override;

private:
    struct CallbackData
    {
        const FrequencySpectrum* spectrum;
    };

    static ImPlotPoint get_indexed_frequency_bin(
            int index,
            void* user_data
    ) noexcept;

    void update_bounding_box(const FrequencySpectrum& spectrum) noexcept;
    void update_bounding_box() noexcept;

    const FrequencySpectrum* get_spectra(std::shared_ptr<Signal> signal);

    /**
     * The maximum bounding box of a DFT plot.
     */
    ImPlotRect bounding_box{
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::lowest(),
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::lowest(),
    };

    std::unordered_map<Signal::id_type, std::unique_ptr<FrequencySpectrum>> spectra_cache;

    const std::string panel_name = "Signal DFT Preview";

    ImPlotSpec plotting_spec_2d;
    Worker& parent_worker;
    Project* active_project = nullptr;
};

} // namespace echomap

#endif // ECHOMAP_SIGNALDFTPANEL_HPP

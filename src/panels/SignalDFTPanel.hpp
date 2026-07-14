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

class EchoMap;
class Worker;

/**
 * Provides an IPanel to display and interact with previews of Signal frequency spectra (i.e., Signal DFTs).
 */
class SignalDFTPanel : public IPanel
{
public:
    explicit SignalDFTPanel(
            Worker& parent_worker,
            EchoMap& app,
            const Project* initial_project = nullptr
    );

    void draw() noexcept override;

    const char* get_imgui_name() const noexcept override;

    void set_active_project(const Project* new_active_project) noexcept override;

    void handle(DFTResult& result) override;

private:
    struct CallbackData
    {
        const FrequencySpectrum* spectrum;
    };

    static ImPlotPoint get_indexed_frequency_bin(
            int index,
            void* user_data
    ) noexcept;

    void draw_options_section() noexcept;
    void draw_preview_section() noexcept;

    void update_bounding_box(const FrequencySpectrum& spectrum) noexcept;
    void update_bounding_box() noexcept;

    const FrequencySpectrum* get_spectra(
            std::shared_ptr<Signal> signal,
            FrequencySpectrum::WindowFunction window_function
    );

    /**
     * The maximum bounding box of a DFT plot.
     */
    ImPlotRect bounding_box{
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::lowest(),
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::lowest(),
    };

    const std::string panel_name = "Signal DFT Preview";

    ImPlotSpec plotting_spec_2d;
    Worker& parent_worker;
    const Project* active_project = nullptr;
    EchoMap& app;

    static constexpr std::array<FrequencySpectrum::WindowFunction, 3> all_window_functions{
            FrequencySpectrum::WindowFunction::Identity,
            FrequencySpectrum::WindowFunction::Hann,
            FrequencySpectrum::WindowFunction::Hamming
    };

    struct SpectrumCacheEntry
    {
        std::array<std::unique_ptr<FrequencySpectrum>, all_window_functions.size()> spectra;
        std::array<bool, all_window_functions.size()> is_pending{};
    };

    /**
     * The spectra cache maps time-series Signal IDs to the corresponding group of DFT spectra.
     */
    std::unordered_map<Signal::id_type, SpectrumCacheEntry> spectra_cache;

    std::array<std::string, all_window_functions.size()> window_function_names;
    FrequencySpectrum::WindowFunction selected_window_function = all_window_functions.front();
};

} // namespace echomap

#endif // ECHOMAP_SIGNALDFTPANEL_HPP

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

#include <sigc++/scoped_connection.h>

#include "../objects/FrequencySpectrum.hpp"
#include "../objects/Signal.hpp"
#include "IPanel.hpp"

namespace echomap
{

class WorkerResultDespatcher;
class DFTResult;
class EchoMap;
class Worker;

/**
 * Provides an IPanel to display and interact with previews of Signal frequency spectra (i.e., Signal DFTs).
 */
class SignalDFTPanel : public IPanel
{
public:
    /**
     * Create a new SignalDFTPanel to display DFTs of Signal waveforms in the frequency domain.
     *
     * The SignalDFTPanel observes the LoadProjectResult message and consumes the DFTResult message.
     *
     * @param parent_worker The Worker to receive ITask commands over the command bus.
     * @param despatcher The despatcher to expose the result buses.
     * @param app The parent application instance.
     * @param initial_project An optional initial Project for the IPanel to display.
     */
    explicit SignalDFTPanel(
            Worker* parent_worker,
            WorkerResultDespatcher& despatcher,
            EchoMap* app,
            const Project* initial_project = nullptr
    );

    void draw() noexcept override;

    const char* get_imgui_name() const noexcept override;

private:
    struct CallbackData
    {
        const FrequencySpectrum* spectrum;
        int index_offset;
    };

    static ImPlotPoint get_indexed_frequency_bin(
            int index,
            void* user_data
    ) noexcept;

    void handle_completed_dft(DFTResult&& result);

    void draw_options_section() noexcept;
    void draw_preview_section() noexcept;

    void reset_available_transform_sizes();
    void update_spectrum_bounds(const FrequencySpectrum& spectrum) noexcept;
    void update_spectrum_bounds() noexcept;
    void update_available_sizes(std::uint64_t maximum_sample_count);
    void reset_viewport_bounds() noexcept;

    const FrequencySpectrum* get_spectra(
            std::shared_ptr<Signal> signal,
            WindowFunctions::AllFunctions window_function,
            std::size_t transform_size
    );

    /**
     * The minimal bounding box required to fully contain the DFT spectrum plot.
     */
    ImPlotRect spectrum_bounds{
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::lowest(),
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::lowest(),
    };

    ImPlotRect viewport_bounds; /**< The user-controlled bounding box of the DFT plots. */

    std::string panel_name = "Signal DFT Preview";
    ImPlotSpec plotting_spec_2d;
    Worker* parent_worker;
    const Project* active_project = nullptr;
    EchoMap* app;

    /**
     * A three-way key into the FrequencySpectrum cache.
     */
    struct CacheKey
    {
        Signal::id_type source_id;
        WindowFunctions::AllFunctions window_function;
        std::size_t transform_size;

        bool operator==(const CacheKey& key) const;
    };

    /**
     * A keyed value within the FrequencySpectrum cache, either denoting a pending, failed, or present DFT.
     */
    struct CacheValue
    {
        enum class State : std::uint8_t
        {
            NotRequested,
            Success,
            Pending,
            Failed,
        } status{State::NotRequested};

        std::unique_ptr<FrequencySpectrum> spectrum;
    };

    /**
     * The hash functor for CacheKey.
     */
    struct CacheKeyHash
    {
        [[nodiscard]] std::size_t operator()(const CacheKey& key) const noexcept;

    private:
        static std::size_t combine(
                std::size_t seed,
                std::size_t value
        ) noexcept;
    };

    std::unordered_map<CacheKey, CacheValue, CacheKeyHash> spectra_cache; /**< Cached DFT spectra. */
    bool use_log_scale = false; /**< Should the DFT be plotted with a linear or base-10 logarithmic freq. axis? */
    static constexpr unsigned int default_size_log = 7; /**< Base-2 log of the minimum transform size. */
    std::vector<std::string> available_sizes;           /**< Strings of all available transform sizes. */
    unsigned int selected_size_log = default_size_log;  /**< Base-2 log of selected transform size. */

    std::vector<sigc::scoped_connection> connections;
};

} // namespace echomap

#endif // ECHOMAP_SIGNALDFTPANEL_HPP

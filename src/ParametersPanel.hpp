/**
 * @file
 * @brief WebCFD parameters ImGui panel specification
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#ifndef WEBCFD_PARAMETERSPANEL_HPP
#define WEBCFD_PARAMETERSPANEL_HPP

#include <implot.h>

#include <string>

#include "IPanel.hpp"
#include "WAVData.hpp"

namespace WebCFD
{

/**
 * @todo outdated documentation
 *
 * Defines a Dear ImGui panel to dynamically control simulation parameters.
 *
 * This class contains weak references to callbacks and the mutable SimulationParameters structure persisted by a WebCFD
 * instance. Therefore, the lifetime of the ParametersPanel should not outlive the parent WebCFD instance.
 */
class ParametersPanel final : public IPanel
{
public:
    ParametersPanel();

    explicit ParametersPanel(const char * path);

    void update_wav_file(const char * path);

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() override;

private:
    /**
     * A wave file with a downsampled copy for visual rendering.
     */
    struct WAV
    {
        /**
         * Create a new wave audio instance and produce its downsampled variant.
         *
         * @param path The path of the wave file on the file system.
         */
        explicit WAV(const char * path);

        /**
         * Create a new wave audio instance and produce its downsampled variant.
         *
         * @param path The path of the wave file on the file system.
         * @param downsample_factor The factor by which to reduce the sample rate in the downsampled copy.
         */
        explicit WAV(const char * path, float downsample_factor);

        WAVData original;
        WAVData downsampled;

    private:
        static constexpr float default_downsample_factor = 50.0f;
    };

    const std::string panel_name = "Simulation Parameters";

    std::optional<WAV> wav_data;

    ImPlotSpec plotting_spec;
};

} // namespace WebCFD

#endif // WEBCFD_PARAMETERSPANEL_HPP

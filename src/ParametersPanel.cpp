/**
 * @file
 * @brief WebCFD parameters ImGui panel implementation
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#include "ParametersPanel.hpp"

#include <imgui.h>
#include <implot.h>

#include <ranges>

#include "Logger.hpp"

namespace WebCFD
{

ParametersPanel::ParametersPanel()
{
    plotting_spec.Stride = sizeof(WAVData::AudioPoint);
}

ParametersPanel::ParametersPanel(
        const char* const path
)
{
    plotting_spec.Stride = sizeof(WAVData::AudioPoint);
    update_wav_file(path);
}

void ParametersPanel::update_wav_file(
        const char* const path
)
{
    wav_data.reset();
    wav_data.emplace(path);

    LOG_F_INFO("Original data: {} samples", wav_data->original.get_sample_rate());
    LOG_F_INFO("Downsampled data: {} samples", wav_data->downsampled.get_sample_rate());
}

const char* ParametersPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void ParametersPanel::draw()
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::Begin(panel_name.c_str(), nullptr, flags);

    if (wav_data.has_value() && ImPlot::BeginPlot("Waveform")) {
        ImPlot::SetupAxes("Amplitude", "Time");

        std::size_t channel_idx = 0;
        for (const auto& channel : wav_data->downsampled) {
            ImPlot::PlotLine(
                    std::string("Channel " + std::to_string(channel_idx)).c_str(),
                    &channel.front().time,
                    &channel.front().amplitude,
                    static_cast<int>(channel.size()),
                    plotting_spec
            );
            ++channel_idx;
        }

        ImPlot::EndPlot();
    }

    ImGui::End();
}

ParametersPanel::WAV::WAV(
        const char* const path
) :
    original(path),
    downsampled(
            original,
            default_downsample_factor
    )
{
}

ParametersPanel::WAV::WAV(
        const char* const path,
        const float downsample_factor
) :
    original(path),
    downsampled(
            original,
            downsample_factor
    )
{
}

} // namespace WebCFD

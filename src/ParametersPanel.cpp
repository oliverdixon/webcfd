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

ParametersPanel::ParametersPanel(
        std::function<void()> invalidate_layout_callback
) :
    invalidate_layout_callback(std::move(invalidate_layout_callback))
{
    plotting_spec.Stride = sizeof(WAVData::AudioPoint);

    Logger::log_f(Logger::Level::Info, std::source_location::current(), "Original data: {} samples",
        wav_data.get_sample_rate());

    Logger::log_f(Logger::Level::Info, std::source_location::current(), "Downsampled data: {} samples",
        downsampled.get_sample_rate());
}

const char* ParametersPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void ParametersPanel::draw()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar;

    if (requires_repositioning || force_repositioning) {
        /*
         * If the reposition has been requested (or forced by the user), reset the ParametersPanel position to the
         * center of the overall viewport. Note that in the presence of Dear ImGui persistence (typically an 'imgui.ini'
         * file in the CWD, the position will not change unless the repositioning request was forced).
         */

        const ImGuiViewport* const viewport = ImGui::GetMainViewport();
        const ImVec2 center{
                viewport->WorkPos.x + viewport->WorkSize.x / 2.0f,
                viewport->WorkPos.y + viewport->WorkSize.y / 2.0f,
        };

        ImGui::SetNextWindowPos(
                center,
                force_repositioning ? ImGuiCond_Always : ImGuiCond_FirstUseEver,
                ImVec2{.5f, .5f}
        );

        flags |= ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize;

        requires_repositioning = false;
        force_repositioning = false;
    }

    ImGui::Begin(panel_name.c_str(), nullptr, flags);

    if (ImPlot::BeginPlot("Waveform")) {
        ImPlot::SetupAxes("Amplitude", "Time");

        std::size_t channel_idx = 0;
        for (const auto& channel : downsampled) {
            ImPlot::PlotLine(std::string("Channel " + std::to_string(channel_idx)).c_str(), &channel.front().time,
                &channel.front().amplitude, static_cast<int>(channel.size()), plotting_spec);
            ++channel_idx;
        }

        ImPlot::EndPlot();
    }

    ImGui::End();
}

} // namespace WebCFD

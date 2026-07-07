/**
 * @file
 * @brief WebCFD signal waveform preview panel implementation
 * @author Oliver Dixon
 * @date 2026-07-07
 */

#include "SignalWaveformPanel.hpp"

#include "../objects/Project.hpp"

namespace WebCFD
{

SignalWaveformPanel::SignalWaveformPanel(
        Project* const initial_project
) :
    active_project(initial_project)
{
    plotting_spec_2d.Stride = sizeof(Signal::Sample);
}

const char* SignalWaveformPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void SignalWaveformPanel::draw() noexcept
{
    if (ImGui::Begin(panel_name.c_str())) {
        if (active_project == nullptr)
            ImGui::Text("No project is loaded.");
        else if (active_project->get_signal_count() > 0 && ImPlot::BeginAlignedPlots("##WaveformAlignedGroup")) {
            ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

            for (const auto& signal : active_project->observe_signals())
                if (const auto downsampled = get_downsampled_signal(signal); downsampled == nullptr)
                    ImGui::Text("Could not downsample %s due to system error.", signal.get_imgui_name());
                else if (ImPlot::BeginPlot(downsampled->get_imgui_name())) {
                    ImPlot::SetupAxisLinks(ImAxis_X1, &waveform_bounding_cache.X.Min, &waveform_bounding_cache.X.Max);
                    ImPlot::SetupAxisLinks(ImAxis_Y1, &waveform_bounding_cache.Y.Min, &waveform_bounding_cache.Y.Max);
                    ImPlot::PlotLine(
                            "",
                            &downsampled->begin()->time,
                            &downsampled->begin()->amplitude,
                            static_cast<int>(downsampled->get_sample_count()),
                            plotting_spec_2d
                    );

                    ImPlot::EndPlot();
                }

            ImPlot::EndAlignedPlots();
            ImPlot::PopStyleColor();
        }
    }

    ImGui::End();
}

void SignalWaveformPanel::set_active_project(
        Project* const new_active_project
) noexcept
{
    active_project = new_active_project;
}

const Signal* SignalWaveformPanel::get_downsampled_signal(
        const Signal& signal
)
{
    auto downsampled_it = downsample_cache.find(signal.get_id());
    bool success;

    if (downsampled_it == downsample_cache.end()) {

        // This is a bit cryptic - https://stackoverflow.com/a/27960637.
        auto [added_it, was_added] = downsample_cache.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(signal.get_id()),
                std::forward_as_tuple(
                        signal,
                        default_downsample_factor,
                        std::format(
                                "{} ({}x LTTB Preview)",
                                signal.get_name(),
                                static_cast<int>(default_downsample_factor)
                        )
                )
        );

        downsampled_it = added_it;
        success = was_added;

        if (success) {
            const auto local_min = added_it->second.begin()->time;
            const auto local_max = std::prev(added_it->second.end())->time;

            if (local_min < waveform_bounding_cache.X.Min)
                waveform_bounding_cache.X.Min = local_min;

            if (local_max > waveform_bounding_cache.X.Max)
                waveform_bounding_cache.X.Max = local_max;
        }
    } else
        success = true;

    return success ? &downsampled_it->second : nullptr;
}

} // namespace WebCFD

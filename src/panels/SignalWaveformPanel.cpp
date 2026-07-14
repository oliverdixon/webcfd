/**
 * @file
 * @brief EchoMap signal waveform preview panel implementation
 * @author Oliver Dixon
 * @date 2026-07-07
 */

#include "SignalWaveformPanel.hpp"

#include "../Logger.hpp"
#include "../objects/FrequencySpectrumFactory.hpp"
#include "../objects/Project.hpp"
#include "../tasks/DownsampleResult.hpp"
#include "../tasks/DownsampleTask.hpp"
#include "../tasks/Worker.hpp"

namespace echomap
{

SignalWaveformPanel::SignalWaveformPanel(
        Worker& parent_worker,
        Project* const initial_project
) :
    parent_worker(parent_worker),
    active_project(initial_project)
{
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

            for (const auto& signal : active_project->share_signals())
                if (const auto spectrum = get_spectra(signal); spectrum == nullptr)
                    ImGui::Text("Could not downsample %s due to system error.", signal->get_imgui_name());
                else if (ImPlot::BeginPlot(spectrum->get_imgui_name())) {

                    ImPlot::SetupAxes("Frequency (Hz)", "Magnitude");
                    ImPlot::SetupAxisLinks(ImAxis_X1, &waveform_bounding_box.X.Min, &waveform_bounding_box.X.Max);
                    ImPlot::SetupAxisLinks(ImAxis_Y1, &waveform_bounding_box.Y.Min, &waveform_bounding_box.Y.Max);

                    FSCallback callback_data = {.spectrum = spectrum};
                    ImPlot::PlotLineG(
                            "",
                            &SignalWaveformPanel::get_indexed_frequency_bin,
                            &callback_data,
                            static_cast<int>(spectrum->bins.size()),
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
    downsample_cache.clear();
    update_bounding_box();
}

void SignalWaveformPanel::handle(
        DownsampleResult& result
)
{
    auto ds_slot_it = downsample_cache.find(result.get_source_id());
    auto signal = result.take_downsampled();

    if (ds_slot_it == downsample_cache.end()) {
        LOG_F_WARN("Received an unexpected result for the downsampled Signal {}.", signal->get_name());
        const auto signal_name_view = signal->get_name();
        auto [it, success] = downsample_cache.emplace(result.get_source_id(), std::move(signal));

        if (!success) {
            LOG_F_ERROR("Could not store the downsampled Signal {} in the cache.", signal_name_view);
            return;
        }

        ds_slot_it = it;
    } else
        ds_slot_it->second = std::move(signal);

    // Update the bounding box for the signal graphical representation.
    update_bounding_box(*ds_slot_it->second);
}

ImPlotPoint SignalWaveformPanel::get_indexed_signal_point(
        const int index,
        void* const user_data
) noexcept
{
    const auto signal = static_cast<SignalCallback*>(user_data)->signal;
    return {signal->get_time_at_index(index), signal->begin()[index]};
}

ImPlotPoint SignalWaveformPanel::get_indexed_frequency_bin(
        const int index,
        void* const user_data
) noexcept
{
    const auto spectrum = static_cast<FSCallback*>(user_data)->spectrum;
    return {spectrum->bins[index].frequency, spectrum->bins[index].magnitude};
}

void SignalWaveformPanel::update_bounding_box(
        const Signal& signal
)
{
    if (signal.get_sample_count() > 0) {
        if (const auto local_min = signal.get_time_at_index(0); local_min < waveform_bounding_box.X.Min)
            waveform_bounding_box.X.Min = local_min;

        if (const auto local_max = signal.get_time_at_index(signal.get_sample_count() - 1);
            local_max > waveform_bounding_box.X.Max)
            waveform_bounding_box.X.Max = local_max;
    }
}

void SignalWaveformPanel::update_bounding_box(
        const FrequencySpectrum& spectrum
)
{
    if (const auto& bins = spectrum.bins; !bins.empty()) {
        if (const auto local_min_x = bins.front().frequency; local_min_x < waveform_bounding_box.X.Min)
            waveform_bounding_box.X.Min = local_min_x;

        if (const auto local_max_x = bins.back().frequency; local_max_x > waveform_bounding_box.X.Max)
            waveform_bounding_box.X.Max = local_max_x;

        auto local_min_y = bins.front().magnitude;
        for (const auto bin : bins)
            if (bin.magnitude < local_min_y)
                local_min_y = bin.magnitude;

        if (local_min_y < waveform_bounding_box.Y.Min)
            waveform_bounding_box.Y.Min = local_min_y;

        auto local_max_y = bins.front().magnitude;
        for (const auto bin : bins)
            if (bin.magnitude > local_max_y)
                local_max_y = bin.magnitude;

        if (local_max_y > waveform_bounding_box.Y.Max)
            waveform_bounding_box.Y.Max = local_max_y;
    }
}

void SignalWaveformPanel::update_bounding_box()
{
    waveform_bounding_box.X.Min = std::numeric_limits<double>::max();
    waveform_bounding_box.X.Max = std::numeric_limits<double>::lowest();
    waveform_bounding_box.Y.Min = -1.0;
    waveform_bounding_box.Y.Max = 1.0;
}

const Signal* SignalWaveformPanel::get_downsampled_signal(
        std::shared_ptr<Signal> signal
)
{
    assert(signal != nullptr);
    const auto downsampled_it = downsample_cache.find(signal->get_id());

    if (downsampled_it == downsample_cache.end()) {
        /*
         * If the source signal hasn't already been downsampled and cached, submit a job to do it ASAP.
         *
         * The result won't be picked up on this render cycle, so return nullptr to indicate the "Loading" state, but it
         * should come through shortly. We emplace a nullptr in the slot to indicate that the work has been requested,
         * but not yet completed.
         */

        downsample_cache.emplace(signal->get_id(), nullptr);
        parent_worker.submit(std::make_unique<DownsampleTask>(std::move(signal)));
        return nullptr;
    }

    return downsampled_it->second.get();
}

const FrequencySpectrum* SignalWaveformPanel::get_spectra(
        std::shared_ptr<Signal> signal // TODO will need to share ownership with scheduler.
)
{
    assert(signal != nullptr);
    auto spectra_it = spectra_cache.find(signal->get_id());

    if (spectra_it == spectra_cache.end())
        spectra_it =
                spectra_cache.emplace(signal->get_id(), FrequencySpectrumFactory::create_frequency_spectrum(*signal))
                        .first;

    update_bounding_box(*spectra_it->second);
    return spectra_it->second.get();
}

} // namespace echomap

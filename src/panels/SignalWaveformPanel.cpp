/**
 * @file
 * @brief EchoMap signal waveform preview panel implementation
 * @author Oliver Dixon
 * @date 2026-07-07
 */

#include "SignalWaveformPanel.hpp"

#include "../Logger.hpp"
#include "../objects/Project.hpp"
#include "../tasks/DownsampleResult.hpp"
#include "../tasks/DownsampleTask.hpp"
#include "../tasks/Worker.hpp"
#include "../tasks/WorkerResultDespatcher.hpp"

namespace echomap
{

SignalWaveformPanel::SignalWaveformPanel(
        Worker& parent_worker,
        WorkerResultDespatcher& despatcher,
        const Project* const initial_project
) :
    parent_worker(parent_worker),
    active_project(initial_project)
{
    connections.push_back(despatcher.load_project_finished_channel.observe([this](const LoadProjectResult& result) {
        active_project = result.observe_project();
        downsample_cache.clear();
        update_bounding_box();
    }));

    despatcher.downsample_finished_channel.nominate_consumer(
            sigc::mem_fun(*this, &SignalWaveformPanel::handle_downsampled_result)
    );
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
                if (const auto downsampled = get_downsampled_signal(signal); downsampled == nullptr)
                    ImGui::Text("Loading downsampled variant of %s...", signal->get_imgui_name());
                else if (ImPlot::BeginPlot(downsampled->get_imgui_name())) {

                    ImPlot::SetupAxes("Time (seconds)", "Amplitude");
                    ImPlot::SetupAxisLinks(ImAxis_X1, &bounding_box.X.Min, &bounding_box.X.Max);
                    ImPlot::SetupAxisLinks(ImAxis_Y1, &bounding_box.Y.Min, &bounding_box.Y.Max);

                    CallbackData callback_data = {.signal = downsampled};
                    ImPlot::PlotLineG(
                            "",
                            &SignalWaveformPanel::get_indexed_signal_point,
                            &callback_data,
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

void SignalWaveformPanel::handle_downsampled_result(
        DownsampleResult&& result
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
    const auto signal = static_cast<CallbackData*>(user_data)->signal;
    return {signal->get_time_at_index(index), signal->begin()[index]};
}

void SignalWaveformPanel::update_bounding_box(
        const Signal& signal
) noexcept
{
    if (signal.get_sample_count() > 0) {
        if (const auto local_min = signal.get_time_at_index(0); local_min < bounding_box.X.Min)
            bounding_box.X.Min = local_min;

        if (const auto local_max = signal.get_time_at_index(signal.get_sample_count() - 1);
            local_max > bounding_box.X.Max)
            bounding_box.X.Max = local_max;
    }
}

void SignalWaveformPanel::update_bounding_box() noexcept
{
    bounding_box.X.Min = std::numeric_limits<double>::max();
    bounding_box.X.Max = std::numeric_limits<double>::lowest();
    bounding_box.Y.Min = -1.0;
    bounding_box.Y.Max = 1.0;
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

} // namespace echomap

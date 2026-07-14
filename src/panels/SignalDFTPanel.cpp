/**
 * @file
 *
 * SignalDFTPanel implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#include "SignalDFTPanel.hpp"

#include "../objects/FrequencySpectrum.hpp"
#include "../objects/Project.hpp"
#include "../tasks/DFTTask.hpp"
#include "../tasks/DFTResult.hpp"
#include "../tasks/Worker.hpp"
#include "../Logger.hpp"

namespace echomap
{

SignalDFTPanel::SignalDFTPanel(
        Worker& parent_worker,
        const Project* const initial_project
) :
    parent_worker(parent_worker),
    active_project(initial_project)
{
}

void SignalDFTPanel::draw() noexcept
{
    if (ImGui::Begin(panel_name.c_str())) {
        if (active_project == nullptr)
            ImGui::Text("No project is loaded.");
        else if (active_project->get_signal_count() > 0 && ImPlot::BeginAlignedPlots("##DFTAlignedGroup")) {
            ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

            for (const auto& signal : active_project->share_signals())
                if (const auto spectrum = get_spectra(signal); spectrum == nullptr)
                    ImGui::Text("Could not take DFT of %s due to system error.", signal->get_imgui_name());
                else if (ImPlot::BeginPlot(spectrum->get_imgui_name())) {

                    ImPlot::SetupAxes("Frequency (Hz)", "Magnitude");
                    ImPlot::SetupAxisLinks(ImAxis_X1, &bounding_box.X.Min, &bounding_box.X.Max);
                    ImPlot::SetupAxisLinks(ImAxis_Y1, &bounding_box.Y.Min, &bounding_box.Y.Max);

                    CallbackData callback_data = {.spectrum = spectrum};
                    ImPlot::PlotLineG(
                            "",
                            &SignalDFTPanel::get_indexed_frequency_bin,
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

const char* SignalDFTPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void SignalDFTPanel::set_active_project(
        const Project* new_active_project
) noexcept
{
    active_project = new_active_project;
    spectra_cache.clear();
    update_bounding_box();
}

void SignalDFTPanel::handle(
        DFTResult& result
)
{
    auto spectrum_slot_it = spectra_cache.find(result.get_source_id());
    auto spectrum = result.take_spectrum();

    if (spectrum_slot_it == spectra_cache.end()) {
        LOG_F_WARN("Received an unexpected result for the DFT of Signal {}.", spectrum->get_name());
        const auto spectrum_name_view = spectrum->get_name();
        auto [it, success] = spectra_cache.emplace(result.get_source_id(), std::move(spectrum));

        if (!success) {
            LOG_F_ERROR("Could not store the DFT of Signal {} in the cache.", spectrum_name_view);
            return;
        }

        spectrum_slot_it = it;
    } else
        spectrum_slot_it->second = std::move(spectrum);

    // Update the bounding box for the graphical representation.
    update_bounding_box(*spectrum_slot_it->second);
}

ImPlotPoint SignalDFTPanel::get_indexed_frequency_bin(
        const int index,
        void* user_data
) noexcept
{
    const auto spectrum = static_cast<CallbackData*>(user_data)->spectrum;
    return {spectrum->bins[index].frequency, spectrum->bins[index].magnitude};
}

void SignalDFTPanel::update_bounding_box(
        const FrequencySpectrum& spectrum
) noexcept
{
    if (const auto& bins = spectrum.bins; !bins.empty()) {
        if (const auto local_min_x = bins.front().frequency; local_min_x < bounding_box.X.Min)
            bounding_box.X.Min = local_min_x;

        if (const auto local_max_x = bins.back().frequency; local_max_x > bounding_box.X.Max)
            bounding_box.X.Max = local_max_x;

        auto local_min_y = bins.front().magnitude;
        for (const auto bin : bins)
            if (bin.magnitude < local_min_y)
                local_min_y = bin.magnitude;

        if (local_min_y < bounding_box.Y.Min)
            bounding_box.Y.Min = local_min_y;

        auto local_max_y = bins.front().magnitude;
        for (const auto bin : bins)
            if (bin.magnitude > local_max_y)
                local_max_y = bin.magnitude;

        if (local_max_y > bounding_box.Y.Max)
            bounding_box.Y.Max = local_max_y;
    }
}

void SignalDFTPanel::update_bounding_box() noexcept
{
    bounding_box.X.Min = std::numeric_limits<double>::max();
    bounding_box.X.Max = std::numeric_limits<double>::lowest();
    bounding_box.Y.Min = std::numeric_limits<double>::max();
    bounding_box.Y.Max = std::numeric_limits<double>::lowest();
}

const FrequencySpectrum* SignalDFTPanel::get_spectra(
        std::shared_ptr<Signal> signal
)
{
    assert(signal != nullptr);
    const auto spectra_it = spectra_cache.find(signal->get_id());

    if (spectra_it == spectra_cache.end()) {
        /*
         * If the source signal hasn't already been transformed and cached, submit a job to do it ASAP.
         *
         * The result won't be picked up on this render cycle, so return nullptr to indicate the "Loading" state, but it
         * should come through shortly. We emplace a nullptr in the slot to indicate that the work has been requested,
         * but not yet completed.
         */

        spectra_cache.emplace(signal->get_id(), nullptr);
        parent_worker.submit(std::make_unique<DFTTask>(std::move(signal)));
        return nullptr;
    }

    return spectra_it->second.get();
}

} // namespace echomap

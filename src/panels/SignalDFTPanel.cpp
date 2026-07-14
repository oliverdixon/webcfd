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
#include "../objects/FrequencySpectrumFactory.hpp"
#include "../objects/Project.hpp"

namespace echomap
{

SignalDFTPanel::SignalDFTPanel(
        Worker& parent_worker,
        Project* const initial_project
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
        Project* new_active_project
) noexcept
{
    active_project = new_active_project;
    spectra_cache.clear();
    update_bounding_box();
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
    auto spectra_it = spectra_cache.find(signal->get_id());

    if (spectra_it == spectra_cache.end())
        spectra_it =
                spectra_cache.emplace(signal->get_id(), FrequencySpectrumFactory::create_frequency_spectrum(*signal))
                        .first;

    update_bounding_box(*spectra_it->second);
    return spectra_it->second.get();
}

} // namespace echomap

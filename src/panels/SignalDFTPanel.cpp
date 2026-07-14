/**
 * @file
 *
 * SignalDFTPanel implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#include "SignalDFTPanel.hpp"

#include "../EchoMap.hpp"
#include "../Logger.hpp"
#include "../objects/FrequencySpectrum.hpp"
#include "../objects/Project.hpp"
#include "../tasks/DFTResult.hpp"
#include "../tasks/DFTTask.hpp"
#include "../tasks/Worker.hpp"

namespace echomap
{

SignalDFTPanel::SignalDFTPanel(
        Worker& parent_worker,
        EchoMap& app,
        const Project* const initial_project
) :
    parent_worker(parent_worker),
    active_project(initial_project),
    app(app)
{
    for (std::size_t window_idx = 0; window_idx < all_window_functions.size(); ++window_idx)
        window_function_names[window_idx] =
                FrequencySpectrum::get_window_function_name(all_window_functions[window_idx]);
}

void SignalDFTPanel::draw() noexcept
{
    if (ImGui::Begin(panel_name.c_str())) {
        if (active_project == nullptr)
            ImGui::Text("No project is loaded.");
        else if (!active_project->get_signal_count())
            ImGui::Text("No signals are loaded.");
        else {
            draw_options_section();
            draw_preview_section();
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
    auto spectrum = result.take_spectrum();

    if (const auto group_it = spectra_cache.find(result.get_source_id()); group_it == spectra_cache.end())
        LOG_F_WARN("Dropping an unexpected result for the DFT of Signal {}.", spectrum->get_name());
    else {
        const auto idx = std::to_underlying(spectrum->preprocessor);

        group_it->second.is_pending[idx] = false;
        group_it->second.spectra[idx] = std::move(spectrum);

        update_bounding_box(*group_it->second.spectra[idx]);
    }
}

ImPlotPoint SignalDFTPanel::get_indexed_frequency_bin(
        const int index,
        void* user_data
) noexcept
{
    const auto spectrum = static_cast<CallbackData*>(user_data)->spectrum;
    return {spectrum->bins[index].frequency, spectrum->bins[index].magnitude};
}

void SignalDFTPanel::draw_options_section() noexcept
{
    ImGui::SeparatorText("DFT Configuration");

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Window Function");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());

    if (auto combo_selected_idx = std::to_underlying(selected_window_function);
        ImGui::BeginCombo("##DFTOptionsWindowFunction", window_function_names[combo_selected_idx].c_str())) {
        for (unsigned int item_idx = 0; item_idx < window_function_names.size(); ++item_idx) {
            const auto is_selected = item_idx == combo_selected_idx;
            if (ImGui::Selectable(window_function_names[item_idx].c_str(), is_selected) &&
                combo_selected_idx != item_idx) {
                combo_selected_idx = item_idx;
                selected_window_function = static_cast<FrequencySpectrum::WindowFunction>(combo_selected_idx);
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
        app.increment_forced_frames(4);
    }
}

void SignalDFTPanel::draw_preview_section() noexcept
{
    ImGui::SeparatorText("DFT Previews");

    if (ImPlot::BeginAlignedPlots("##DFTAlignedGroup")) {
        ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        for (const auto& signal : active_project->share_signals())
            if (const auto spectrum = get_spectra(signal, selected_window_function); spectrum == nullptr)
                ImGui::Text(
                        "Loading DFT of %s with the %s window function...",
                        signal->get_imgui_name(),
                        FrequencySpectrum::get_window_function_name(selected_window_function).c_str()
                );
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
        std::shared_ptr<Signal> signal,
        const FrequencySpectrum::WindowFunction window_function
)
{
    assert(signal != nullptr);

    const auto signal_id = signal->get_id();

    auto [group_it, inserted] = spectra_cache.try_emplace(signal_id);

    auto& entry = group_it->second;
    const auto idx = std::to_underlying(window_function);

    if (entry.spectra[idx] != nullptr)
        return entry.spectra[idx].get();

    if (!entry.is_pending[idx]) {
        entry.is_pending[idx] = true;
        parent_worker.submit(std::make_unique<DFTTask>(std::move(signal), window_function));
    }

    return nullptr;
}

} // namespace echomap

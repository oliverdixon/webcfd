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

        group_it->second.status[idx] = CacheEntryState::Success;
        group_it->second.spectra[idx] = std::move(spectrum);

        update_bounding_box(*group_it->second.spectra[idx]);
    }
}

ImPlotPoint SignalDFTPanel::get_indexed_frequency_bin(
        int index,
        void* const user_data
) noexcept
{
    const auto info = static_cast<CallbackData*>(user_data);
    index += info->index_offset;
    return {info->spectrum->cbegin()[index].frequency, info->spectrum->cbegin()[index].magnitude};
}

void SignalDFTPanel::draw_options_section() noexcept
{
    ImGui::SeparatorText("DFT Configuration");

    if (ImGui::BeginTable("##DFTOptionsTable", 2, table_flags)) {
        ImGui::TableSetupColumn("##DFTOptionsTableLabel", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("##DFTOptionsTableControl", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::TextUnformatted("Input Window Function");
        ImGui::TableNextColumn();

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
            app.increment_forced_frames();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::TextUnformatted("Logarithmic Frequency Scale");
        ImGui::TableNextColumn();

        if (ImGui::Checkbox("##DFTOptionsLogScale", &use_log_scale))
            update_bounding_box();

        ImGui::EndTable();
    }
}

void SignalDFTPanel::draw_preview_section() noexcept
{
    ImGui::SeparatorText("DFT Previews");

    if (ImPlot::BeginAlignedPlots("##DFTAlignedGroup")) {
        ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        for (const auto& signal :
             active_project->share_signals() | std::views::filter([](const std::shared_ptr<Signal>& candidate) {
                 return candidate->is_uniformly_sampled();
             })) {

            if (const auto spectrum = get_spectra(signal, selected_window_function); spectrum != nullptr) {
                // Case 1: we got a spectrum immediately.
                if (ImPlot::BeginPlot(spectrum->get_imgui_name())) {
                    ImPlot::SetupAxes("Frequency (Hz)", "Magnitude");
                    ImPlot::SetupAxisScale(ImAxis_X1, use_log_scale ? ImPlotScale_Log10 : ImPlotScale_Linear);
                    ImPlot::SetupAxisLinks(ImAxis_X1, &bounding_box.X.Min, &bounding_box.X.Max);
                    ImPlot::SetupAxisLinks(ImAxis_Y1, &bounding_box.Y.Min, &bounding_box.Y.Max);

                    int plottable_bin_count = static_cast<int>(spectrum->get_bin_count());
                    CallbackData callback_data = {.spectrum = spectrum, .index_offset = 0};

                    if (plottable_bin_count > 0 && use_log_scale) {
                        // If we're plotting on the log scale, discount the DC component if it exists.
                        --plottable_bin_count;
                        callback_data.index_offset = 1;
                    }

                    ImPlot::PlotLineG(
                            "",
                            &SignalDFTPanel::get_indexed_frequency_bin,
                            &callback_data,
                            plottable_bin_count,
                            plotting_spec_2d
                    );

                    ImPlot::EndPlot();
                }
            } else {
                // Case 2: we didn't get one immediately. Either it's pending, or it failed.
                const auto status =
                        spectra_cache[signal->get_id()].status[std::to_underlying(selected_window_function)];

                if (status == CacheEntryState::Pending)
                    ImGui::Text(
                            "Loading DFT of %s with the %s window function...",
                            signal->get_imgui_name(),
                            FrequencySpectrum::get_window_function_name(selected_window_function).c_str()
                    );
                else if (status == CacheEntryState::Failed) {
                    const auto function_name = FrequencySpectrum::get_window_function_name(selected_window_function);
                    LOG_F_ERROR(
                            "The front-end is reporting that {} DFT with {} failed.",
                            signal->get_name(),
                            function_name
                    );

                    ImGui::Text(
                            "Failed to load DFT of %s with the %s function. This is a bug.",
                            signal->get_imgui_name(),
                            function_name.c_str()
                    );
                }
            }
        }

        ImPlot::EndAlignedPlots();
        ImPlot::PopStyleColor();
    }
}

void SignalDFTPanel::update_bounding_box(
        const FrequencySpectrum& spectrum
) noexcept
{
    using BoundType = decltype(bounding_box.X.Min);

    if (spectrum.get_bin_count() == 0)
        return;

    if (use_log_scale) {
        if (spectrum.get_bin_count() > 1) {
            if (const auto x_min = static_cast<BoundType>(std::next(spectrum.cbegin())->frequency); x_min > 0.0)
                bounding_box.X.Min = std::min(x_min, bounding_box.X.Min);
        }
    } else
        bounding_box.X.Min = std::min(static_cast<BoundType>(spectrum.get_minimum_frequency()), bounding_box.X.Min);

    bounding_box.X.Max = std::max(static_cast<BoundType>(spectrum.get_maximum_frequency()), bounding_box.X.Max);
    bounding_box.Y.Min = std::min(static_cast<BoundType>(spectrum.get_minimum_magnitude()), bounding_box.Y.Min);
    bounding_box.Y.Max = std::max(static_cast<BoundType>(spectrum.get_maximum_magnitude()), bounding_box.Y.Max);
}

void SignalDFTPanel::update_bounding_box() noexcept
{
    bounding_box.X.Min = std::numeric_limits<double>::max();
    bounding_box.X.Max = std::numeric_limits<double>::lowest();
    bounding_box.Y.Min = std::numeric_limits<double>::max();
    bounding_box.Y.Max = std::numeric_limits<double>::lowest();

    for (const auto& group : spectra_cache | std::views::values)
        for (const auto& spectrum : group.spectra)
            if (spectrum != nullptr)
                update_bounding_box(*spectrum);
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

    if (entry.status[idx] != CacheEntryState::Pending) {
        entry.status[idx] = CacheEntryState::Pending;
        parent_worker.submit(std::make_unique<DFTTask>(std::move(signal), window_function));
    }

    return nullptr;
}

} // namespace echomap

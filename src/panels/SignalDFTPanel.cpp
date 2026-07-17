/**
 * @file
 *
 * SignalDFTPanel implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#include "SignalDFTPanel.hpp"

#include <bit>

#include "../EchoMap.hpp"
#include "../Logger.hpp"
#include "../VariantHelpers.hpp"
#include "../objects/FrequencySpectrum.hpp"
#include "../objects/Project.hpp"
#include "../signals/Worker.hpp"
#include "../signals/results/DFTResult.hpp"
#include "../signals/tasks/DFTTask.hpp"

namespace
{

using namespace echomap;

constexpr auto window_function_names =
        variant_helpers::variant_name_array<WindowFunctions::AllFunctions, WindowFunctions::NameGetter>;

} // namespace

namespace echomap
{

SignalDFTPanel::SignalDFTPanel(
        Worker* parent_worker,
        WorkerResultDespatcher& despatcher,
        EchoMap* app,
        const Project* const initial_project
) :
    parent_worker(parent_worker),
    active_project(initial_project),
    app(app)
{
    connections.emplace_back(despatcher.load_project_finished_channel.observe([this](const LoadProjectResult& result) {
        active_project = result.observe_project();
        spectra_cache.clear();
        reset_available_transform_sizes();
        update_spectrum_bounds();
        reset_viewport_bounds();
    }));

    connections.emplace_back(despatcher.dft_finished_channel.nominate_consumer(
            sigc::mem_fun(*this, &SignalDFTPanel::handle_completed_dft)
    ));

    reset_available_transform_sizes();
}

void SignalDFTPanel::draw() noexcept
{
    if (ImGui::Begin(panel_name.c_str())) {
        if (active_project == nullptr)
            ImGui::Text("No project is loaded.");
        else if (active_project->get_signal_count() == 0u)
            ImGui::Text("No signals are loaded.");
        else {
            std::uint64_t max_sample_count = 0;
            for (const auto& signal : active_project->observe_signals())
                max_sample_count = std::max(max_sample_count, signal.get_sample_count());
            update_available_sizes(max_sample_count);

            draw_configuration_section();
            draw_preview_section();
        }
    }

    ImGui::End();
}

const char* SignalDFTPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void SignalDFTPanel::handle_completed_dft(
        DFTResult&& result
)
{
    auto spectrum = std::move(result).take_spectrum();

    if (spectrum == nullptr) {
        LOG_WARN("Dropping a null DFT result.");
        return;
    }

    const CacheKey key{
        .source_id = result.get_source_id(),
            .window_function = spectrum->preprocessor,
            .transform_size = result.get_transform_size(),
    };

    const auto cache_slot_it = spectra_cache.find(key);
    if (cache_slot_it == spectra_cache.end()) {
        LOG_F_WARN("Dropping an unexpected result for the DFT of Signal {}.", spectrum->get_name());
        return;
    }

    cache_slot_it->second.status = CacheValue::State::Success;
    cache_slot_it->second.spectrum = std::move(spectrum);

    const bool result_is_currently_visible = key.window_function.index() == selected_window.index() &&
                                             key.transform_size == (std::size_t{1} << selected_size_log);

    if (result_is_currently_visible) {
        const auto had_no_visible_spectrum =
                spectrum_bounds.X.Min > spectrum_bounds.X.Max || spectrum_bounds.Y.Min > spectrum_bounds.Y.Max;

        update_spectrum_bounds(*cache_slot_it->second.spectrum);

        if (had_no_visible_spectrum)
            reset_viewport_bounds();

        app->increment_forced_frames();
    }
}

ImPlotPoint SignalDFTPanel::get_indexed_frequency_bin(
        int index,
        void* const user_data
) noexcept
{
    const auto* const info = static_cast<CallbackData*>(user_data);
    index += info->index_offset;
    return {info->spectrum->cbegin()[index].frequency, info->spectrum->cbegin()[index].magnitude};
}

void SignalDFTPanel::draw_configuration_section() noexcept
{
    ImGui::SeparatorText("DFT Configuration");

    if (ImGui::BeginTable("##DFTOptionsTable", 2, table_flags)) {
        ImGui::TableSetupColumn("##DFTOptionsTableLabel", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("##DFTOptionsTableControl", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        draw_configuration_window_function();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        draw_configuration_transform_size();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        draw_configuration_scale_type();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        draw_configuration_preview_actions();

        ImGui::EndTable();
    }
}

void SignalDFTPanel::draw_configuration_window_function() noexcept
{
    ImGui::TextUnformatted("Input Window Function");
    ImGui::TableNextColumn();

    ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());

    if (auto selected_idx = selected_window.index();
        /*
         * Disabled string_view::data warnings: we know that the views were constructed from string literals, hence
         * NULL-terminated. Dear ImGui provides no API for specifying the lengths of the expected data, so we can rely
         * on the termination here.
         */

        // NOLINTNEXTLINE(*-suspicious-stringview-data-usage)
        ImGui::BeginCombo("##DFTOptionsWindowFunction", window_function_names[selected_idx].data())) {
        for (std::size_t item_idx = 0; item_idx < window_function_names.size(); ++item_idx) {
            const auto is_selected = item_idx == selected_idx;

            // NOLINTNEXTLINE(*-suspicious-stringview-data-usage)
            if (ImGui::Selectable(window_function_names[item_idx].data(), is_selected) && selected_idx != item_idx) {
                selected_idx = item_idx;

                try {
                    selected_window = variant_helpers::variant_from_index<WindowFunctions::AllFunctions>(selected_idx);
                } catch (const std::out_of_range&) {
                    LOG_WARN("Invalid window function index was selected. Resetting to the default.");
                    selected_window = WindowFunctions::Constant{};
                }

                update_spectrum_bounds();
                reset_viewport_bounds();
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
        app->increment_forced_frames();
    }
}

void SignalDFTPanel::draw_configuration_transform_size() noexcept
{
    ImGui::TextUnformatted("Transform Size");
    ImGui::TableNextColumn();

    ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());

    if (ImGui::BeginCombo("##DFTOptionsTransformSize", available_sizes[selected_size_log - default_size_log].c_str())) {
        for (unsigned int item_idx = 0; item_idx < available_sizes.size(); ++item_idx) {
            const auto is_selected = item_idx == selected_size_log - default_size_log;
            if (ImGui::Selectable(available_sizes[item_idx].c_str(), is_selected) &&
                selected_size_log != item_idx + default_size_log) {

                // The selected size has changed.
                selected_size_log = item_idx + default_size_log;
                update_spectrum_bounds();
                reset_viewport_bounds();
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
        app->increment_forced_frames();
    }
}

void SignalDFTPanel::draw_configuration_scale_type() noexcept
{
    ImGui::TextUnformatted("Logarithmic Frequency Scale");
    ImGui::TableNextColumn();

    if (ImGui::Checkbox("##DFTOptionsLogScale", &use_log_scale)) {
        update_spectrum_bounds();
        reset_viewport_bounds();
    }
}

void SignalDFTPanel::draw_configuration_preview_actions() noexcept
{
    ImGui::TextUnformatted("Preview Actions");
    ImGui::TableNextColumn();

    if (ImGui::Button("Reset Viewports##DFTOptionsResetViewport"))
        reset_viewport_bounds();
    ImGui::SameLine();
    if (ImGui::Button("Reset Cached DFTs##DFTOptionsResetCache")) {
        spectra_cache.clear();
        update_spectrum_bounds();
        reset_viewport_bounds();
        app->increment_forced_frames();
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
             }))
            draw_preview_of_signal(signal);

        ImPlot::EndAlignedPlots();
        ImPlot::PopStyleColor();
    }
}

void SignalDFTPanel::draw_preview_of_signal(
        std::shared_ptr<Signal> signal
) noexcept
{
    const auto* const name = signal->get_imgui_name();
    if (const auto* const spectrum =
                get_spectra(std::move(signal), selected_window, std::size_t{1} << selected_size_log);
        spectrum != nullptr) {

        // Case 1: we got a spectrum immediately.
        if (ImPlot::BeginPlot(spectrum->get_imgui_name())) {
            ImPlot::SetupAxes("Frequency (Hz)", "Magnitude (dBfs)");
            ImPlot::SetupAxisScale(ImAxis_X1, use_log_scale ? ImPlotScale_Log10 : ImPlotScale_Linear);
            ImPlot::SetupAxisLinks(ImAxis_X1, &viewport_bounds.X.Min, &viewport_bounds.X.Max);
            ImPlot::SetupAxisLinks(ImAxis_Y1, &viewport_bounds.Y.Min, &viewport_bounds.Y.Max);

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

    } else
        // Case 2: we didn't get one immediately. Either it's pending, or it failed.
        // TODO: indicate failure here as well as loading.
        ImGui::Text(
                "Loading DFT of %s with the %s window function...",
                name,
                // NOLINTNEXTLINE(*-suspicious-stringview-data-usage)
                window_function_names[selected_window.index()].data()
        );
}

void SignalDFTPanel::reset_available_transform_sizes()
{
    // TODO: what if we have signals, but they all have less than 128 samples?

    available_sizes.clear();
    available_sizes.push_back(std::to_string(std::size_t{1} << default_size_log));
    selected_size_log = default_size_log;
}

void SignalDFTPanel::update_spectrum_bounds(
        const FrequencySpectrum& spectrum
) noexcept
{
    using BoundType = decltype(spectrum_bounds.X.Min);

    const auto bin_count = static_cast<std::ptrdiff_t>(spectrum.get_bin_count());
    if (bin_count == 0)
        return;

    const ptrdiff_t first_bin_idx = use_log_scale && bin_count > 1 ? 1 : 0;
    if (first_bin_idx >= bin_count)
        return;

    spectrum_bounds.X.Min =
            std::min(static_cast<BoundType>(spectrum.cbegin()[first_bin_idx].frequency), spectrum_bounds.X.Min);
    spectrum_bounds.X.Max =
            std::max(static_cast<BoundType>(spectrum.cbegin()[bin_count - 1].frequency), spectrum_bounds.X.Max);

    for (auto bin_idx = first_bin_idx; bin_idx < bin_count; ++bin_idx) {
        const auto magnitude = static_cast<BoundType>(spectrum.cbegin()[bin_idx].magnitude);
        spectrum_bounds.Y.Min = std::min(magnitude, spectrum_bounds.Y.Min);
        spectrum_bounds.Y.Max = std::max(magnitude, spectrum_bounds.Y.Max);
    }
}

void SignalDFTPanel::update_spectrum_bounds() noexcept
{
    spectrum_bounds.X.Min = std::numeric_limits<double>::max();
    spectrum_bounds.X.Max = std::numeric_limits<double>::lowest();
    spectrum_bounds.Y.Min = std::numeric_limits<double>::max();
    spectrum_bounds.Y.Max = std::numeric_limits<double>::lowest();

    const auto selected_transform_size = std::size_t{1} << selected_size_log;

    for (const auto& [key, value] : spectra_cache)
        if (value.spectrum != nullptr && key.window_function.index() == selected_window.index() &&
            key.transform_size == selected_transform_size)
            update_spectrum_bounds(*value.spectrum);
}

void SignalDFTPanel::update_available_sizes(
        const std::uint64_t maximum_sample_count
)
{
    if (available_sizes.empty())
        reset_available_transform_sizes();

    if (maximum_sample_count > std::uint64_t{1} << (available_sizes.size() + default_size_log - 1)) {
        /*
         * If the maximum sample count can support a large transform size than currently advertised, re-create the list.
         * (Yes, we could be smarter here and just append the tail.)
         */
        constexpr std::uint64_t minimum_transform_size = std::uint64_t{1} << default_size_log;
        const std::size_t maximum_transform_size =
                std::max(minimum_transform_size, std::bit_floor(maximum_sample_count));

        available_sizes.clear();
        for (std::uint64_t size = minimum_transform_size; size <= maximum_transform_size; size <<= std::uint64_t{1}) {
            available_sizes.push_back(std::to_string(size));
            if (size > std::numeric_limits<std::uint64_t>::max() / 2)
                break;
        }
    }

    // Once we know the correct maximum transform size, bound the selection to the maximum available.
    if (selected_size_log >= available_sizes.size() + default_size_log) {
        selected_size_log =
                static_cast<unsigned int>(available_sizes.size() + default_size_log - 1);
    }
}

void SignalDFTPanel::reset_viewport_bounds() noexcept
{
    viewport_bounds = spectrum_bounds;

    if (use_log_scale && viewport_bounds.X.Min <= 0.0)
        viewport_bounds.X.Min = 1.0;

    if (viewport_bounds.X.Min >= viewport_bounds.X.Max) {
        viewport_bounds.X.Min = use_log_scale ? 1.0 : 0.0;
        viewport_bounds.X.Max = 1.0;
    }

    if (viewport_bounds.Y.Min >= viewport_bounds.Y.Max) {
        viewport_bounds.Y.Min = 0.0;
        viewport_bounds.Y.Max = 1.0;
    }
}

const FrequencySpectrum* SignalDFTPanel::get_spectra(
        std::shared_ptr<Signal> signal,
        const WindowFunctions::AllFunctions window_function,
        const std::size_t transform_size
)
{
    assert(signal != nullptr);

    const CacheKey key{
            .source_id = signal->get_id(),
            .window_function = window_function,
            .transform_size = transform_size,
    };

    auto& entry = spectra_cache[key];
    if (entry.spectrum != nullptr)
        return entry.spectrum.get();

    if (entry.status != CacheValue::State::Pending) {
        entry.status = CacheValue::State::Pending;
        parent_worker->submit(std::make_unique<DFTTask>(std::move(signal), window_function, transform_size));
    }

    return nullptr;
}

bool SignalDFTPanel::CacheKey::operator==(
        const CacheKey& key
) const
{
    return key.source_id == source_id && key.transform_size == transform_size &&
           key.window_function.index() == window_function.index();
}

std::size_t SignalDFTPanel::CacheKeyHash::operator()(
        const CacheKey& key
) const noexcept
{
    std::size_t seed = std::hash<Signal::id_type>{}(key.source_id);
    seed = combine(seed, std::hash<std::size_t>{}(key.window_function.index()));
    return combine(seed, std::hash<std::size_t>{}(key.transform_size));
}

std::size_t SignalDFTPanel::CacheKeyHash::combine(
        const std::size_t seed,
        const std::size_t value
) noexcept
{
    return seed ^ value + 0x9e3779b97f4a7c15ULL + (seed << 6U) + (seed >> 2U);
}

} // namespace echomap

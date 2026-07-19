/**
 * @file
 *
 * IndividualUploadModal implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-17
 */

#include "IndividualUploadModal.hpp"

#include "../EchoMap.hpp"
#include "../actions/ActionController.hpp"
#include "../objects/Project.hpp"

namespace echomap
{

IndividualUploadModal::IndividualUploadModal(
        EchoMap* const app,
        const Project* const project
) :
    app(app),
    project(project)
{
}

void IndividualUploadModal::draw() noexcept // TODO remove noexcepts where necessary.
{
    if (project->unloaded_signals.empty())
        return;

    if (std::exchange(should_open, false))
        ImGui::OpenPopup("Upload External Files##UploadExternalModal");

    ImGui::SetNextWindowSize(default_modal_size, ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(
                "Upload External Files##UploadExternalModal",
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize
        )) {
        ImGui::PushID("UploadExternalModal");

        draw_preamble();

        // The remaining number of signals for which there is no given path in VFS, decremented as we enumerate.
        auto unmapped_count = project->unloaded_signals.size();

        if (ImGui::BeginTable("##UploadTable", 5, table_flags)) {
            ImGui::TableSetupColumn("##UploadButton", ImGuiTableColumnFlags_WidthFixed, button_size.x);
            ImGui::TableSetupColumn("Indicated Path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Signal Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Channel Number", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Given Path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (const auto& [external_path, mapping_info] : project->unloaded_signals)
                if (draw_table_entry(
                            external_path,
                            mapping_info.first,
                            std::views::transform(mapping_info.second, [](const std::unique_ptr<SignalFactory>& ptr) {
                                return ptr.get();
                            })
                    ))

                    --unmapped_count;

            ImGui::EndTable();
        }

        draw_buttons(unmapped_count == 0);

        ImGui::PopID();
        ImGui::EndPopup();
    }
}

const char* IndividualUploadModal::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void IndividualUploadModal::reshow() noexcept
{
    should_open = true;
}

void IndividualUploadModal::draw_preamble() const noexcept
{
    ImGui::TextWrapped(
                "%s contains references to externally sourced signals. Browser security requires that each externally "
                "sourced file is uploaded separately.",
                project->get_imgui_name()
        );

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextWrapped(
            "Upload all missing files to complete the mapping, and press Continue.\n\nCancelling the operation "
            "will abort the load and revert back to the previously loaded project, if applicable."
    );

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void IndividualUploadModal::draw_buttons(
        const bool are_all_mapped
)
{
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Cancel", button_size)) {
        // TODO send an abort message to the project. Will need a new notification.
        ImGui::CloseCurrentPopup();
        should_open = false;
    }

    ImGui::SameLine();

    if (are_all_mapped) {
        if (ImGui::Button("Continue", button_size)) {
            app->submit_lightweight_task(CompleteProjectLoadNotification(project->get_id()));
            ImGui::CloseCurrentPopup();
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("Continue", button_size);
        ImGui::SetItemTooltip("To continue, provide mappings for all unloaded externally sourced signals.");
        ImGui::EndDisabled();
    }
}

bool IndividualUploadModal::draw_table_entry(
        const std::filesystem::path& external_path,
        const std::optional<std::filesystem::path>& vfs_path,
        SignalFactoryRange auto&& factories
) const noexcept
{
    ImGui::PushID(external_path.c_str());
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    const auto& padding = ImGui::GetStyle().CellPadding;
    ImGui::SetCursorPos({ImGui::GetCursorPosX() - padding.x, ImGui::GetCursorPosY() - padding.y});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, upload_button_frame_padding);

    if (ImGui::Button("Upload", ImVec2(button_size.x + 2 * padding.x, button_size.y)))
        ActionController::register_vfs_mapping(project->get_id(), external_path);

    ImGui::PopStyleVar();

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(external_path.c_str());

    bool is_first = true;
    bool is_mapped = false;

    for (const auto signal_factory : factories | std::views::filter([](auto&& ptr) {
                                         return ptr;
                                     })) {
        if (!is_first)
            ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(2);

        const auto& signal = signal_factory->observe_signal();
        ImGui::PushID(signal.get_imgui_name());

        ImGui::TextUnformatted(signal.get_imgui_name());
        ImGui::TableNextColumn();
        const auto formatted_channel_num = std::to_string(signal.observe_source()->channel);
        ImGui::TextUnformatted(formatted_channel_num.c_str());
        ImGui::TableNextColumn();

        if (is_first) {
            const auto& given_path = vfs_path;
            if (given_path.has_value()) {
                ImGui::TextUnformatted(given_path->c_str());
                is_mapped = true;
            } else
                ImGui::TextUnformatted("Not provided");

            is_first = false;
        }

        ImGui::PopID();
    }

    ImGui::PopID();
    return is_mapped;
}

} // namespace echomap

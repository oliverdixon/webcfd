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
#include "../Logger.hpp"
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

void IndividualUploadModal::draw() noexcept
{
    // TODO refactor monster.

    ImGui::OpenPopup("Upload External Files##UploadExternalModal");
    if (ImGui::BeginPopupModal(
                "Upload External Files##UploadExternalModal",
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize
        )) {
        ImGui::PushID("UploadExternalModal");

        ImGui::TextWrapped(
                "%s contains references to externally sources signals. Browser security requires that each externally "
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

        // The remaining number of signals for which there is no given path in VFS, decremented as we enumerate.
        auto unmapped_count = project->unloaded_signals.size();

        if (ImGui::BeginTable("##UploadTable", 3, table_flags)) {
            ImGui::TableSetupColumn("##UploadButton", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Indicated Path", ImGuiTableColumnFlags_WidthStretch);
            // ImGui::TableSetupColumn("Signal Name", ImGuiTableColumnFlags_WidthStretch); // TODO
            // ImGui::TableSetupColumn("Channel Number", ImGuiTableColumnFlags_WidthStretch); // TODO
            ImGui::TableSetupColumn("Given Path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            std::size_t row_entry = 0;

            for (const auto& [external_path, mapping_info] : project->unloaded_signals) {
                ImGui::PushID(static_cast<int>(row_entry++));
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (ImGui::Button("Upload"))
                    ActionController::register_vfs_mapping(project->get_id(), external_path);

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(external_path.c_str());
                // ImGui::TableNextColumn();
                // ImGui::TextUnformatted(partial_signal.observe_source()->path.c_str());
                // ImGui::TableNextColumn();
                // ImGui::Text("%ld", partial_signal.observe_source()->channel);
                ImGui::TableNextColumn();

                const auto& given_path = mapping_info.first;

                if (given_path.has_value()) {
                    ImGui::TextUnformatted(given_path->c_str());
                    --unmapped_count;
                } else
                    ImGui::TextUnformatted("Not provided");

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        constexpr float button_width = 80.0f;

        if (ImGui::Button("Cancel", ImVec2(button_width, 0.0f)))
            ImGui::CloseCurrentPopup();

        ImGui::SameLine();

        if (unmapped_count == 0) {
            if (ImGui::Button("Continue", ImVec2(button_width, 0.0f))) {
                app->submit_lightweight_task(CompleteProjectLoadNotification(project->get_id()));
                ImGui::CloseCurrentPopup();
            }
        } else {
            ImGui::BeginDisabled();
            ImGui::Button("Continue", ImVec2(button_width, 0.0f));
            ImGui::SetItemTooltip("To continue, provide mappings for all unloaded externally sourced signals.");
            ImGui::EndDisabled();
        }

        ImGui::PopID();
        ImGui::EndPopup();
    }
}

const char* IndividualUploadModal::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

} // namespace echomap

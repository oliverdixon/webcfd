/**
 * @file
 *
 * IndividualUploadModal implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-17
 */

#include "IndividualUploadModal.hpp"

#include "../objects/Project.hpp"

namespace echomap
{

IndividualUploadModal::IndividualUploadModal(
        const Project* const project
) :
    project(project)
{
}

void IndividualUploadModal::draw() noexcept
{
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
                "Upload all missing files to complete the mapping, and press Continue. Selecting Postpone will continue"
                "to load the project, but signals and channel mappings will be unavailable until all uploads are"
                "complete."
        );

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTable("##UploadTable", 4, table_flags)) {
            ImGui::TableSetupColumn("##UploadButton", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Signal Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Indicated Path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Upload Status", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (int i = 0; i < 6; ++i) {
                ImGui::PushID(i);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::Button("Upload");
                ImGui::TableNextColumn();

                ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
                ImGui::Text("A");
                ImGui::TableNextColumn();
                ImGui::Text("//");
                ImGui::TableNextColumn();
                ImGui::Text("Not provided");

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        constexpr float button_width = 80.0f;

        if (ImGui::Button("Postpone", ImVec2(button_width, 0.0f)))
            ImGui::CloseCurrentPopup();

        ImGui::SameLine();

        if (ImGui::Button("Continue", ImVec2(button_width, 0.0f)))
            ImGui::CloseCurrentPopup();

        ImGui::PopID();
        ImGui::EndPopup();
    }
}

const char* IndividualUploadModal::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

} // namespace echomap

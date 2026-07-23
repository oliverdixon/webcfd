/**
 * @file
 *
 * NativeFileChooser implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-22
 */

#include "NativeFileChooser.hpp"

#include "../utility/Logger.hpp"

namespace echomap
{

NativeFileChooser::NativeFileChooser(
        EchoMap* const app
) :
    file_combo(app),
    panel_name(std::string("Select File to Open") + get_imgui_stable_name()),
    app(app)
{
}

void NativeFileChooser::draw() noexcept
{
    if (std::exchange(should_open, false))
        ImGui::OpenPopup(get_imgui_name());

    ImGui::SetNextWindowSize(default_modal_size, ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(get_imgui_name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushID(get_imgui_name());

        if (file_combo(chosen_path))
            LOG_F_DEBUG("Changed: {}", chosen_path.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Cancel", button_size)) {
            ImGui::CloseCurrentPopup();
            should_open = false;
        }

        ImGui::SameLine();

        if (chosen_path.extension() == expected_extension)
            ImGui::Button("OK", button_size);
        else {
            ImGui::BeginDisabled();
            ImGui::Button("OK", button_size);
            ImGui::SetItemTooltip("To continue, select a JSON EchoMap project file.");
            ImGui::EndDisabled();
        }

        ImGui::PopID();
        ImGui::EndPopup();
    }
}

const char* NativeFileChooser::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void NativeFileChooser::change_active_project(
        const Project* const new_project
)
{
    std::ignore = new_project;
}

const char* NativeFileChooser::get_imgui_stable_name() noexcept
{
    return "###NativeFileChooser";
}

} // namespace echomap

/**
 * @file
 * @brief EchoMap modal error panel implementation
 * @author Oliver Dixon
 * @date 2026-06-30
 */

#include "ErrorModal.hpp"

#include <imgui.h>

#include "../utility/Logger.hpp"

namespace echomap
{

ErrorModal::ErrorModal() :
    panel_name(std::string("Error!") + get_imgui_stable_name())
{
}

void ErrorModal::raise_error(
        const std::string_view message
) noexcept
{
    try {
        prefix = message;
    } catch (const std::exception&) {
        LOG_ERROR("Could not allocate memory to display modal on UI.");
        is_raised = false;
        return;
    }

    is_raised = true;
}

void ErrorModal::raise_error(
        const std::string_view new_prefix,
        const std::runtime_error& exception
) noexcept
{
    try {
        detail = exception.what();
        prefix = new_prefix;
    } catch (const std::exception&) {
        LOG_ERROR("Could not allocate memory to display modal on UI.");
        is_raised = false;
        return;
    }

    is_raised = true;
}

void ErrorModal::draw() noexcept
{
    if (!is_raised)
        return;

    ImGui::OpenPopup(get_imgui_name());
    if (ImGui::BeginPopupModal(get_imgui_name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted(prefix.c_str());
        ImGui::Separator();

        if (detail.has_value()) {
            ImGui::TextWrapped("%s", detail->c_str());
            ImGui::Separator();
        }

        if (ImGui::Button("Dismiss", button_size)) {
            is_raised = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

const char* ErrorModal::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void ErrorModal::change_active_project(
        const Project* const new_project
)
{
    std::ignore = new_project;
}

const char* ErrorModal::get_imgui_stable_name() noexcept
{
    return "###ErrorModal";
}

} // namespace echomap

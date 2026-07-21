//
// Created by owd on 07/07/2026.
//

#include "ChannelMappingPanel.hpp"

#include "../EchoMap.hpp"
#include "../objects/Project.hpp"
#include "../objects/Sensor.hpp"
#include "../objects/Signal.hpp"

namespace echomap
{

ChannelMappingPanel::ChannelMappingPanel(
        WorkerResultDespatcher& despatcher,
        EchoMap* app,
        const Project* const initial_project
) :
    app(app),
    active_project(initial_project)
{
    connections.emplace_back(despatcher.load_project_finished_channel.observe([this](const LoadProjectResult& result) {
        active_project = result.observe_project();
    }));
}

const char* ChannelMappingPanel::get_imgui_name() const noexcept
{
    return panel_name.c_str();
}

void ChannelMappingPanel::draw() noexcept
{
    if (ImGui::Begin(panel_name.c_str())) {
        if (active_project == nullptr)
            ImGui::Text("No project is loaded.");
        else {
            ImGui::SeparatorText("Create Channel Mapping");
            draw_new_channel_mapping();

            // If a new mapping has been fully described, add it and prompt for another.
            if (new_entry_cache.signal != nullptr && new_entry_cache.sensor != nullptr) {
                app->notify(AddChannelMappingNotification(
                        active_project->get_id(),
                        new_entry_cache.signal->get_id(),
                        new_entry_cache.sensor->get_id()
                ));

                new_entry_cache.signal = nullptr;
                new_entry_cache.sensor = nullptr;
            }

            ImGui::SeparatorText("Existing Channel Mapping");
            draw_existing_channel_mapping();
        }
    }

    ImGui::End();
}

void ChannelMappingPanel::draw_new_channel_mapping() noexcept
{
    // TODO refactor monster.

    if (ImGui::BeginTable("##NewChannelMapping", 2, table_flags)) {
        ImGui::TableSetupColumn("Signal", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Sensor", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        bool need_to_force = false;

        // Prompt for the associated signal.
        ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
        if (ImGui::BeginCombo(
                    "##NewAssociationSignal",
                    new_entry_cache.signal == nullptr ? "Select signal..." : new_entry_cache.signal->get_imgui_name(),
                    0
            )) {
            for (const auto& signal : active_project->observe_signals()) {
                const bool is_selected = new_entry_cache.signal == nullptr ? false : signal == *new_entry_cache.signal;

                // Checks if something has changed (thus current value needs updating).
                if (ImGui::Selectable(signal.get_imgui_name(), is_selected))
                    new_entry_cache.signal = &signal;

                // Checks if the current item is selected.
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
            need_to_force = true;
        }

        ImGui::TableNextColumn();

        // Prompt for the associated sensor.
        ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
        if (ImGui::BeginCombo(
                    "##NewAssociationSensor",
                    new_entry_cache.sensor == nullptr ? "Select sensor..." : new_entry_cache.sensor->get_imgui_name(),
                    0
            )) {
            for (const auto& sensor : active_project->observe_sensors()) {
                const bool is_selected = new_entry_cache.sensor == nullptr ? false : sensor == *new_entry_cache.sensor;

                if (ImGui::Selectable(sensor.get_imgui_name(), is_selected))
                    new_entry_cache.sensor = &sensor;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
            need_to_force = true;
        }

        ImGui::EndTable();

        if (need_to_force)
            app->increment_forced_frames();
    }
}

void ChannelMappingPanel::draw_existing_channel_mapping() const noexcept
{
    if (ImGui::BeginTable("##ExistingChannelMapping", 2, table_flags)) {
        ImGui::TableSetupColumn("Signal", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Sensor", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        // Display existing associations.
        for (const auto& [signal, sensor] : active_project->observe_associations()) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            ImGui::TextUnformatted(signal.get_imgui_name());
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-std::numeric_limits<float>::min());
            ImGui::TextUnformatted(sensor.get_imgui_name());
        }

        ImGui::EndTable();
    }
}

} // namespace echomap

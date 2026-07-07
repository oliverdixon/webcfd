/**
 * @file
 * @brief WebCFD main viewport panel specification
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#ifndef WEBCFD_VIEWPORTPANEL_HPP
#define WEBCFD_VIEWPORTPANEL_HPP

#include <implot3d.h>

#include <string>

#include "../objects/Project.hpp"
#include "ErrorModal.hpp"
#include "IPanel.hpp"

namespace WebCFD
{

class ViewportPanel final : public IPanel
{
public:
    explicit ViewportPanel(Project* initial_project = nullptr);

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() noexcept override;

    void set_active_project(Project* new_active_project) noexcept override;

private:
    struct AddChannelMappingRowCache
    {
        const Signal * selected_signal = nullptr;
        const Sensor * selected_sensor = nullptr;
    } new_mapping_cache;

    void draw_sensor_geometry() noexcept;

    void draw_channel_mappings() noexcept;
    void draw_new_channel_mapping() noexcept;
    void draw_existing_channel_mapping() const noexcept;

    std::vector<ImU32> sensor_colours;

    mutable ErrorModal error_modal;

    const std::string panel_name = "Manager";
    static constexpr auto table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg;

    ImPlot3DSpec plotting_spec_3d;

    Project* active_project = nullptr;
};

} // namespace WebCFD

#endif // WEBCFD_VIEWPORTPANEL_HPP

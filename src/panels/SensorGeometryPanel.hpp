/**
 * @file
 * @brief EchoMap sensor geometry panel specification
 * @author Oliver Dixon
 * @date 2026-07-07
 */

#ifndef ECHOMAP_SENSORGEOMETRYPANEL_HPP
#define ECHOMAP_SENSORGEOMETRYPANEL_HPP

#include <implot3d.h>

#include <string>

#include "IPanel.hpp"

namespace echomap
{

class EchoMap;
class Project;

/**
 * Provides a panel for defining and reviewing (in a 3D plot) positions of loaded Sensor objects in the active Project.
 */
class SensorGeometryPanel final : public IPanel
{
public:
    explicit SensorGeometryPanel(
            EchoMap& app,
            const Project* initial_project = nullptr
    );

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() noexcept override;

    void set_active_project(const Project* new_active_project) noexcept override;

private:
    void recache_sensor_colours() noexcept;
    void draw_geometry_summary() noexcept;
    void draw_geometry_plot() const noexcept;

    const std::string panel_name = "Sensor Geometry";

    std::vector<ImU32> sensor_colours;
    ImPlot3DSpec plotting_spec_3d;
    const Project* active_project = nullptr;
    EchoMap& app;
};

} // namespace echomap

#endif // ECHOMAP_SENSORGEOMETRYPANEL_HPP

/**
 * @file
 * @brief EchoMap sensor geometry panel specification
 * @author Oliver Dixon
 * @date 2026-07-07
 */

#ifndef ECHOMAP_SENSORGEOMETRYPANEL_HPP
#define ECHOMAP_SENSORGEOMETRYPANEL_HPP

#include <implot3d.h>
#include <sigc++/scoped_connection.h>

#include <string>

#include "IPanel.hpp"

namespace echomap
{

class EchoMap;
class Project;
class WorkerResultDespatcher;

/**
 * Provides a panel for defining and reviewing (in a 3D plot) positions of loaded Sensor objects in the active Project.
 */
class SensorGeometryPanel final : public IPanel
{
public:
    /**
     * Create a new SensorGeometryPanel to plot and control Sensor information.
     *
     * The SensorGeometryPanel observes the LoadProjectResult message.
     *
     * @param despatcher The despatcher to expose the result buses.
     * @param app The parent application instance.
     * @param initial_project An optional initial Project for the IPanel to display.
     */
    explicit SensorGeometryPanel(
            WorkerResultDespatcher& despatcher,
            EchoMap* app,
            const Project* initial_project = nullptr
    );

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() noexcept override;

    static const char* get_imgui_stable_name() noexcept;

private:
    void recache_sensor_colours() noexcept;
    void draw_geometry_summary() noexcept;
    void draw_geometry_plot() const noexcept;

    std::string panel_name;

    std::vector<ImU32> sensor_colours;
    ImPlot3DSpec plotting_spec_3d;
    const Project* active_project = nullptr;
    EchoMap* app;
    std::vector<sigc::scoped_connection> connections;
};

} // namespace echomap

#endif // ECHOMAP_SENSORGEOMETRYPANEL_HPP

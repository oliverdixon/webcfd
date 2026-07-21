//
// Created by owd on 25/06/2026.
//

#ifndef ECHOMAP_PROJECTPANEL_HPP
#define ECHOMAP_PROJECTPANEL_HPP

#include <sigc++/scoped_connection.h>

#include <string>

#include "IPanel.hpp"

namespace echomap
{

class WorkerResultDespatcher;

class ProjectPanel final : public IPanel
{
public:
    /**
     * Create a new ProjectPanel to display Project metadata.
     *
     * The SensorGeometryPanel observes the LoadProjectResult message.
     *
     * @param despatcher The despatcher to expose the result buses.
     * @param initial_project An optional initial Project for the IPanel to describe.
     */
    explicit ProjectPanel(
            WorkerResultDespatcher& despatcher,
            const Project* initial_project = nullptr
    );

    void draw() noexcept override;

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    static const char* get_imgui_stable_name() noexcept;

private:
    std::string panel_name;

    const Project* active_project = nullptr;
    std::vector<sigc::scoped_connection> connections;
};

} // namespace echomap

#endif // ECHOMAP_PROJECTPANEL_HPP

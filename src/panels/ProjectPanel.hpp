//
// Created by owd on 25/06/2026.
//

#ifndef ECHOMAP_PROJECTPANEL_HPP
#define ECHOMAP_PROJECTPANEL_HPP

#include "../objects/Project.hpp"
#include "IPanel.hpp"

namespace echomap
{

class ProjectPanel final : public IPanel
{
public:
    explicit ProjectPanel(const Project* initial_project = nullptr);

    void draw() noexcept override;

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void set_active_project(const Project* new_active_project) noexcept override;

private:
    const std::string panel_name = "Project Explorer";

    const Project* active_project = nullptr;
};

} // namespace echomap

#endif // ECHOMAP_PROJECTPANEL_HPP

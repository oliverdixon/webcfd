//
// Created by owd on 25/06/2026.
//

#ifndef ECHOMAP_MENUPANEL_HPP
#define ECHOMAP_MENUPANEL_HPP

#include <string>

#include "IPanel.hpp"

namespace echomap
{

class MenuPanel final : public IPanel
{
public:
    void draw() noexcept override;

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void set_active_project(const Project* new_active_project) noexcept override;

private:
    std::string panel_name = "Menu";
};

} // namespace echomap

#endif // ECHOMAP_MENUPANEL_HPP

//
// Created by owd on 25/06/2026.
//

#ifndef WEBCFD_MENUPANEL_HPP
#define WEBCFD_MENUPANEL_HPP

#include <string>

#include "IPanel.hpp"

namespace WebCFD
{

class MenuPanel final : public IPanel
{
public:
    void draw() noexcept override;

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

private:
    std::string panel_name = "Menu";
};

} // namespace WebCFD

#endif // WEBCFD_MENUPANEL_HPP

//
// Created by owd on 25/06/2026.
//

#ifndef WEBCFD_PROJECTPANEL_HPP
#define WEBCFD_PROJECTPANEL_HPP

#include "IPanel.hpp"

namespace WebCFD
{

class ProjectPanel final : public IPanel
{
public:
    void draw() noexcept override;

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

private:
    std::string panel_name = "Unnamed Project";
};

} // namespace WebCFD

#endif // WEBCFD_PROJECTPANEL_HPP

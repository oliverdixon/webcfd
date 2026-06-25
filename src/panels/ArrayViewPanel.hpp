/**
 * @file
 * @brief WebCFD sensor array position view implementation
 * @author Oliver Dixon
 * @date 2026-06-24
 */

#ifndef WEBCFD_ARRAYVIEWPANEL_HPP
#define WEBCFD_ARRAYVIEWPANEL_HPP

#include <implot3d.h>

#include <string>

#include "../audio/Sensor.hpp"
#include "IPanel.hpp"

namespace WebCFD
{

class ArrayViewPanel final : public IPanel
{
public:
    explicit ArrayViewPanel(std::string panel_name, const std::vector<Sensor>& sensors);

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

    void draw() noexcept override;

private:
    const std::string panel_name;
    const std::vector<Sensor>& sensors;
    ImPlot3DSpec plotting_spec;
};

} // namespace WebCFD

#endif // WEBCFD_ARRAYVIEWPANEL_HPP

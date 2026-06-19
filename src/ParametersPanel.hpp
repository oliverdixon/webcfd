//
// Created by owd on 04/06/2026.
//

#ifndef WEBCFD_PARAMETERSPANEL_HPP
#define WEBCFD_PARAMETERSPANEL_HPP

#include "IPanel.hpp"
#include "SimulationParameters.hpp"

namespace WebCFD
{

class ParametersPanel final : public IPanel
{
public:
    explicit ParametersPanel(SimulationParameters& parameters, std::function<void()> invalidate_layout_callback);

    const char* get_imgui_name() const noexcept override;

    void draw() override;

private:
    const std::string panel_name = "Simulation Parameters";

    bool requires_repositioning = true;
    bool force_repositioning = false;

    const SimulationParameters default_parameters{};
    SimulationParameters& parameters;
    std::function<void()> invalidate_layout_callback;
};

} // namespace WebCFD

#endif // WEBCFD_PARAMETERSPANEL_HPP

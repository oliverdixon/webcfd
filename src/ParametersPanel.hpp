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
    explicit ParametersPanel(SimulationParameters& parameters);

    void draw() override;

private:
    SimulationParameters& parameters;
};

} // namespace WebCFD

#endif // WEBCFD_PARAMETERSPANEL_HPP

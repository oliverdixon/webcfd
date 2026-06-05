//
// Created by owd on 05/06/2026.
//

#ifndef WEBCFD_SIMULATIONPARAMETERS_H
#define WEBCFD_SIMULATIONPARAMETERS_H

namespace WebCFD
{

struct alignas(16) SimulationParameters
{
    float colour[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
};

} // namespace WebCFD

#endif // WEBCFD_SIMULATIONPARAMETERS_H

//
// Created by owd on 05/06/2026.
//

#ifndef WEBCFD_SIMULATIONPARAMETERS_H
#define WEBCFD_SIMULATIONPARAMETERS_H

namespace WebCFD
{

struct alignas(16) SimulationParameters
{
    float viewport[4] = { 0.0f, 1.0f, 1.0f, 1.0f };

    float controls[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    float colour_a[4] = { 0.5f, 0.5f, 0.5f, 0.0f };
    float colour_b[4] = { 0.5f, 0.5f, 0.5f, 0.0f };
    float colour_c[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
    float colour_d[4] = { 0.0f, 0.3f, 0.7f, 0.0f };
};

} // namespace WebCFD

#endif // WEBCFD_SIMULATIONPARAMETERS_H

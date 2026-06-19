//
// Created by owd on 05/06/2026.
//

#ifndef WEBCFD_SIMULATIONPARAMETERS_H
#define WEBCFD_SIMULATIONPARAMETERS_H

namespace WebCFD
{

struct alignas(16) ShaderVec4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

struct alignas(16) ShaderVec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float _padding = 0.0f;
};

struct alignas(16) SimulationParameters {
    ShaderVec4 controls;   // speed, intensity, warp, scale

    ShaderVec3 colour_a;
    ShaderVec3 colour_b;
    ShaderVec3 colour_c;
    ShaderVec3 colour_d;
};

} // namespace WebCFD

#endif // WEBCFD_SIMULATIONPARAMETERS_H

//
// Created by owd on 05/06/2026.
//

#ifndef WEBCFD_SIMULATIONPARAMETERS_H
#define WEBCFD_SIMULATIONPARAMETERS_H

#include <random>

namespace WebCFD
{

struct alignas(
        16
) ShaderVec4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

struct alignas(
        16
) ShaderVec3
{
    ShaderVec3(
            const float x,
            const float y,
            const float z
    ) noexcept :
        x(x),
        y(y),
        z(z)
    {
    }
 float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

private:
    float padding = 0.0f;
};

struct alignas(
        16
) SimulationParameters
{
    SimulationParameters() noexcept :
        colour_a(
                distribution(generator),
                distribution(generator),
                distribution(generator)
        ),
        colour_b(
                distribution(generator),
                distribution(generator),
                distribution(generator)
        ),
        colour_c(
                distribution(generator),
                distribution(generator),
                distribution(generator)
        ),
        colour_d(
                distribution(generator),
                distribution(generator),
                distribution(generator)
        )
    {
    }

    ShaderVec4 controls = {
            .x = 2.0f, // Speed
            .y = 1.5f, // Intensity
            .z = 1.5f, // Warp
            .w = 1.5f  // Scale
    };

    ShaderVec3 colour_a;
    ShaderVec3 colour_b;
    ShaderVec3 colour_c;
    ShaderVec3 colour_d;

private:
    static std::random_device random_device;
    static std::mt19937 generator;
    static std::uniform_real_distribution<float> distribution;
};

} // namespace WebCFD

#endif // WEBCFD_SIMULATIONPARAMETERS_H

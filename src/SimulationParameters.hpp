/**
 * @file
 * @brief WebCFD parameters structure specification
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#ifndef WEBCFD_SIMULATIONPARAMETERS_H
#define WEBCFD_SIMULATIONPARAMETERS_H

#include <random>

namespace WebCFD
{

/**
 * A 16-byte-aligned four-dimensional real vector.
 */
struct alignas(
        16
) ShaderVec4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

/**
 * A 16-byte-aligned three-dimensional real vector.
 *
 * Because WebGPU shaders align three-dimensional float-32 vectors on a 64-byte boundary, this structure contains
 * padding such that <code>sizeof(ShaderVec3) == sizeof(ShaderVec4)</code>.
 */
struct alignas(
        16
) ShaderVec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

private:
    float padding = 0.0f;
};

/**
 * WebCFD simulation parameters (animation and colouring) relevant to all shaders.
 *
 * Optional RNG services are provided through static members.
 */
struct alignas(
        16
) SimulationParameters
{

    /**
     * Initialise a new SimulationParameters instance, optionally randomising the initial colourways.
     *
     * @param randomise Should the colourways be randomised?
     */
    explicit SimulationParameters(
            const bool randomise = false
    ) noexcept
    {
        if (randomise) {
            colour_a.x = distribution(generator);
            colour_a.y = distribution(generator);
            colour_a.z = distribution(generator);

            colour_b.x = distribution(generator);
            colour_b.y = distribution(generator);
            colour_b.z = distribution(generator);

            colour_c.x = distribution(generator);
            colour_c.y = distribution(generator);
            colour_c.z = distribution(generator);

            colour_d.x = distribution(generator);
            colour_d.y = distribution(generator);
            colour_d.z = distribution(generator);
        }
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

/**
 * @file
 * @brief WebCFD parameters structure implementation
 * @author Oliver Dixon
 * @date 2026-06-20
 */

#include "SimulationParameters.hpp"

namespace WebCFD
{

std::random_device SimulationParameters::random_device;
std::mt19937 SimulationParameters::generator(random_device());
std::uniform_real_distribution<float> SimulationParameters::distribution(
        0.0f,
        1.0f
);

} // namespace WebCFD

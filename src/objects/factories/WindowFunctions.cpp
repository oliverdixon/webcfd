/**
 * @file
 *
 * WindowFunctions implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-16
 */

#include "WindowFunctions.hpp"

#include <cmath>

namespace echomap
{

float WindowFunctions::Hann::operator()(
        const std::size_t index,
        const std::size_t size
) noexcept
{
    if (size <= 1)
        return 1.0f;

    const auto sine = std::sin(std::numbers::pi_v<float> * static_cast<float>(index) / static_cast<float>(size - 1));
    return sine * sine;
}

float WindowFunctions::Hamming::operator()(
        const std::size_t index,
        const std::size_t size
) noexcept
{
    if (size <= 1)
        return 1.0f;

    constexpr float hamming_ratio = 25.0f / 46;
    return hamming_ratio - (1.0f - hamming_ratio) * std::cos(
                                                            2.0f * std::numbers::pi_v<float> *
                                                            static_cast<float>(index) / static_cast<float>(size - 1)
                                                    );
}

float WindowFunctions::Constant::operator()(
        std::size_t /*index*/,
        std::size_t /*size*/
) noexcept
{
    return 1.0f;
}

float WindowFunctions::Bartlett::operator()(
        const std::size_t index,
        const std::size_t size
) noexcept
{
    if (size <= 1)
        return 1.0f;

    const auto centre = static_cast<float>(size - 1) / 2.0f;
    return (2.0f / static_cast<float>(size - 1)) * (centre - std::abs(static_cast<float>(index) - centre));
}

float WindowFunctions::Blackman::operator()(
        const std::size_t index,
        const std::size_t size
) noexcept
{
    if (size <= 1)
        return 1.0f;

    const auto phase = 2.0f * std::numbers::pi_v<float> * static_cast<float>(index) / static_cast<float>(size - 1);
    return 0.42f - 0.5f * std::cos(phase) + 0.08f * std::cos(2.0f * phase);
}

float WindowFunctions::BlackmanHarris::operator()(
        const std::size_t index,
        const std::size_t size
) noexcept
{
    if (size <= 1)
        return 1.0f;

    // Disabled warnings: a{0,1,2,3} are standard names in the literature describing Blackman-Harris.

    constexpr auto a0 = 0.35875f; // NOLINT(*-identifier-length)
    constexpr auto a1 = 0.48829f; // NOLINT(*-identifier-length)
    constexpr auto a2 = 0.14128f; // NOLINT(*-identifier-length)
    constexpr auto a3 = 0.01168f; // NOLINT(*-identifier-length)

    const auto phase = 2.0f * std::numbers::pi_v<float> * static_cast<float>(index) / static_cast<float>(size - 1);
    return a0 - a1 * std::cos(phase) + a2 * std::cos(2.0f * phase) - a3 * std::cos(3.0f * phase);
}

float WindowFunctions::Welch::operator()(
        const std::size_t index,
        const std::size_t size
) noexcept
{
    if (size <= 1)
        return 1.0f;

    const auto centre = static_cast<float>(size - 1) / 2.0f;
    const auto x = (static_cast<float>(index) - centre) / centre;
    return 1.0f - x * x;
}

} // namespace echomap

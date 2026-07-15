/**
 * @file
 *
 * FrequencySpectrum implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#include "FrequencySpectrum.hpp"

#include <utility>

namespace echomap
{

template <> constexpr std::string_view Object<FrequencySpectrum>::class_name = "FreqSpec";

std::string FrequencySpectrum::get_window_function_name(
        const WindowFunction function
) noexcept
{
    switch (function) {
    case WindowFunction::Hamming:
        return "Hamming";
    case WindowFunction::Hann:
        return "Hann";
    case WindowFunction::Identity:
        return "Identity";
    }

    std::unreachable();
}

decltype(FrequencySpectrum::bins)::const_iterator FrequencySpectrum::begin() const
{
    return bins.begin();
}

decltype(FrequencySpectrum::bins)::const_iterator FrequencySpectrum::end() const
{
    return bins.end();
}

decltype(FrequencySpectrum::bins)::const_iterator FrequencySpectrum::cbegin() const noexcept
{
    return bins.cbegin();
}

decltype(FrequencySpectrum::bins)::const_iterator FrequencySpectrum::cend() const noexcept
{
    return bins.cend();
}

std::size_t FrequencySpectrum::get_bin_count() const noexcept
{
    return bins.size();
}

float FrequencySpectrum::get_minimum_frequency() const noexcept
{
    return minimum_frequency;
}

float FrequencySpectrum::get_maximum_frequency() const noexcept
{
    return maximum_frequency;
}

float FrequencySpectrum::get_minimum_magnitude() const noexcept
{
    return minimum_magnitude;
}

float FrequencySpectrum::get_maximum_magnitude() const noexcept
{
    return maximum_magnitude;
}

FrequencySpectrum::FrequencySpectrum(
        const WindowFunction preprocessor,
        const std::string_view name
) :
    Object(name),
    preprocessor(preprocessor)
{
}

void FrequencySpectrum::reserve_bins(
        const std::size_t bin_count
)
{
    bins.reserve(bin_count);
}

void FrequencySpectrum::emplace_bin(
        float frequency,
        float magnitude,
        float phase
)
{
    bins.emplace_back(frequency, magnitude, phase);

    minimum_frequency = std::min(frequency, minimum_frequency);
    maximum_frequency = std::max(frequency, maximum_frequency);

    minimum_magnitude = std::min(magnitude, minimum_magnitude);
    maximum_magnitude = std::max(magnitude, maximum_magnitude);
}

} // namespace echomap

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

template <> constexpr std::string Object<FrequencySpectrum>::class_name = "FreqSpec";

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

FrequencySpectrum::FrequencySpectrum(
        const WindowFunction preprocessor,
        const std::string_view name
) :
    Object(name),
    preprocessor(preprocessor)
{
}

} // namespace echomap

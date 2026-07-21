/**
 * @file
 *
 * FrequencySpectrum specification
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#ifndef ECHOMAP_FREQUENCYSPECTRUM_HPP
#define ECHOMAP_FREQUENCYSPECTRUM_HPP

#include <vector>

#include "Object.hpp"
#include "factories/WindowFunctions.hpp"

namespace echomap
{

class FrequencySpectrumFactory;

/**
 * A Signal sampled in the frequency domain.
 */
class FrequencySpectrum : public Object<FrequencySpectrum>
{
public:
    /**
     * A bin within the spectrum, denoting the magnitude (and optionally, complex phase) of a Signal at a given
     * frequency.
     */
    struct Bin
    {
        float frequency; /**< The frequency quantised by the Bin, in Hz. */
        float magnitude; /**< The magnitude of the frequency, in dBfs (decibels relative to full scale). */
        float phase;     /**< The complex phase of the DFT value, in radians. */
    };

private:
    std::vector<Bin> bins;

public:
    /**
     * @todo Private
     */
    WindowFunctions::AllFunctions preprocessor;

    [[nodiscard]] decltype(bins)::const_iterator begin() const;
    [[nodiscard]] decltype(bins)::const_iterator end() const;
    [[nodiscard]] decltype(bins)::const_iterator cbegin() const noexcept;
    [[nodiscard]] decltype(bins)::const_iterator cend() const noexcept;

    [[nodiscard]] std::size_t get_bin_count() const noexcept;

    [[nodiscard]] float get_minimum_frequency() const noexcept;
    [[nodiscard]] float get_maximum_frequency() const noexcept;
    [[nodiscard]] float get_minimum_magnitude() const noexcept;
    [[nodiscard]] float get_maximum_magnitude() const noexcept;

private:
    friend FrequencySpectrumFactory;

    explicit FrequencySpectrum(
            WindowFunctions::AllFunctions preprocessor,
            std::string_view name
    );

    void reserve_bins(std::size_t bin_count);

    void emplace_bin(
            float frequency,
            float magnitude,
            float phase
    );

    float minimum_frequency = std::numeric_limits<float>::max();
    float maximum_frequency = std::numeric_limits<float>::min();
    float minimum_magnitude = std::numeric_limits<float>::max();
    float maximum_magnitude = std::numeric_limits<float>::min();
};

template <> constexpr std::string_view Object<FrequencySpectrum>::class_name = "FreqSpec";

} // namespace echomap

#endif // ECHOMAP_FREQUENCYSPECTRUM_HPP

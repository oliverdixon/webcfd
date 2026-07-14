/**
 * @file
 *
 * FrequencySpectrumFactory implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#include "FrequencySpectrumFactory.hpp"

#include <fftw3.h>

#include <cmath>

#include "FrequencySpectrum.hpp"
#include "Signal.hpp"

namespace echomap
{

std::unique_ptr<FrequencySpectrum> FrequencySpectrumFactory::create_frequency_spectrum(
        const Signal& signal
)
{
    if (!signal.is_uniformly_sampled())
        throw std::runtime_error(std::format("Attempted to transform variably sampled {}.", signal.get_name()));

    const auto amplitudes = signal.amplitudes();
    const auto sample_count = amplitudes.size();
    const auto bin_count = sample_count / 2 + 1;
    std::vector<fftwf_complex> coefficients(bin_count);

    const fftwf_plan plan = fftwf_plan_dft_r2c_1d( // NOLINT(*-misplaced-const) - Not misplaced.
            static_cast<int>(sample_count),
            const_cast<float*>(amplitudes.data()),
            coefficients.data(),
            FFTW_ESTIMATE
    );

    if (plan == nullptr)
        throw std::runtime_error("Failed to create FFTW real-to-complex plan.");

    fftwf_execute(plan);
    fftwf_destroy_plan(plan);

    auto spectrum = std::make_unique<FrequencySpectrum>();
    spectrum->bins.reserve(bin_count);

    for (std::size_t bin_idx = 0; bin_idx < bin_count; ++bin_idx) {
        const auto real = coefficients[bin_idx][0];
        const auto imag = coefficients[bin_idx][1];

        const auto is_dc = bin_idx == 0;
        const auto is_nyquist = sample_count % 2 == 0 && bin_idx == sample_count / 2;

        const auto scale = (is_dc || is_nyquist ? 1.0f : 2.0f) / static_cast<float>(sample_count);
        const auto frequency = static_cast<float>(bin_idx) * static_cast<float>(signal.get_sample_rate()) /
                               static_cast<float>(sample_count);
        const auto magnitude = std::sqrt(real * real + imag * imag) * scale;
        const auto phase = std::atan2(imag, real);

        spectrum->bins.emplace_back(frequency, magnitude, phase);
    }

    return std::move(spectrum);
}

} // namespace echomap

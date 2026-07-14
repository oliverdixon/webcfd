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

#include "../FrequencySpectrum.hpp"
#include "../Signal.hpp"
#include "FFTWBuffers.hpp"

namespace echomap
{

std::unique_ptr<FrequencySpectrum> FrequencySpectrumFactory::create_frequency_spectrum(
        const Signal& signal,
        const WindowFunction window_function
)
{
    if (!signal.is_uniformly_sampled())
        throw std::runtime_error(std::format("Attempted to transform variably sampled {}.", signal.get_name()));

    const auto amplitudes = signal.amplitudes();
    const auto sample_count = amplitudes.size();

    // Create FFTW buffers, making a separate input buffer if and only if we're using a non-identity input transform.
    const FFTWBuffers context(
            sample_count,
            window_function == WindowFunction::Identity ? std::make_optional(amplitudes) : std::nullopt
    );

    // Pre-process input as advised by the window function.
    float scale_divisor;
    switch (window_function) {
    case WindowFunction::Identity:
        scale_divisor = static_cast<float>(sample_count);
        break;
    case WindowFunction::Hann:
        scale_divisor = 0.0f;
        for (std::size_t sample_idx = 0; sample_idx < sample_count; ++sample_idx) {
            const auto window = hann_window(sample_idx, sample_count);
            scale_divisor += window;
            // ReSharper disable once CppDFANullDereference - Checked with invariant on FFTWContext::FFTWContext.
            context.input[sample_idx] = amplitudes[sample_idx] * window;
        }

        break;
    case WindowFunction::Hamming:
        scale_divisor = 0.0f;
        for (std::size_t sample_idx = 0; sample_idx < sample_count; ++sample_idx) {
            const auto window = hann_window(sample_idx, sample_count);
            scale_divisor += window;
            // ReSharper disable once CppDFANullDereference - Checked with invariant on FFTWContext::FFTWContext.
            context.input[sample_idx] = amplitudes[sample_idx] * window;
        }

        break;
    }

    // Create a plan and do the FFT.
    const fftwf_plan plan = fftwf_plan_dft_r2c_1d( // NOLINT(*-misplaced-const) - Not misplaced.
            static_cast<int>(sample_count),
            context.input,
            context.coefficients,
            FFTW_ESTIMATE
    );

    fftwf_execute(plan);
    fftwf_destroy_plan(plan);

    // Construct the FrequencySpectrum from the coefficients.
    const auto bin_count = sample_count / 2 + 1;
    auto spectrum = std::make_unique<FrequencySpectrum>();
    spectrum->bins.reserve(bin_count);

    for (std::size_t bin_idx = 0; bin_idx < bin_count; ++bin_idx) {
        // ReSharper disable once CppDFANullDereference - Checked with invariant on FFTWContext::FFTWContext.
        const auto real = context.coefficients[bin_idx][0];
        // ReSharper disable once CppDFANullDereference - Checked with invariant on FFTWContext::FFTWContext.
        const auto imag = context.coefficients[bin_idx][1];

        const auto is_dc = bin_idx == 0;
        const auto is_nyquist = sample_count % 2 == 0 && bin_idx == sample_count / 2;

        // ReSharper disable once CppLocalVariableMightNotBeInitialized - Always initialised by pre-processing step.
        const auto scale = (is_dc || is_nyquist ? 1.0f : 2.0f) / scale_divisor;
        const auto frequency = static_cast<float>(bin_idx) * static_cast<float>(signal.get_sample_rate()) /
                               static_cast<float>(sample_count);
        const auto magnitude = std::sqrt(real * real + imag * imag) * scale;
        const auto phase = std::atan2(imag, real);

        spectrum->bins.emplace_back(frequency, magnitude, phase);
    }

    return std::move(spectrum);
}

Signal::Sample::AmplitudeT FrequencySpectrumFactory::hann_window(
        const std::size_t index,
        const std::size_t size
) noexcept
{
    if (size <= 1)
        return 1.0f;

    const auto sine = std::sin(std::numbers::pi_v<float> * static_cast<float>(index) / static_cast<float>(size - 1));
    return sine * sine;
}

float FrequencySpectrumFactory::hamming_window(
        std::size_t index,
        std::size_t size
) noexcept
{
    if (size <= 1)
        return 1.0f;

    constexpr float hamming = 25 / 46;

    return hamming - (1.0f - hamming) * std::cos(
                                                2.0f * std::numbers::pi_v<float> * static_cast<float>(index) /
                                                static_cast<float>(size - 1)
                                        );
}

} // namespace echomap

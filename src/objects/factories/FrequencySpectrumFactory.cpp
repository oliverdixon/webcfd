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
#include <numbers>

#include "../FrequencySpectrum.hpp"
#include "../Signal.hpp"
#include "FFTWBuffers.hpp"

namespace echomap
{

std::unique_ptr<FrequencySpectrum> FrequencySpectrumFactory::create_frequency_spectrum(
        const Signal& signal,
        const FrequencySpectrum::WindowFunction window_function
)
{
    if (!signal.is_uniformly_sampled())
        throw std::runtime_error(std::format("Attempted to transform variably sampled {}.", signal.get_name()));

    const auto display_name =
            std::format("{} ({} DFT)", signal.get_name(), FrequencySpectrum::get_window_function_name(window_function));
    const auto sample_count = signal.get_sample_count();
    if (sample_count == 0)
        return std::unique_ptr<FrequencySpectrum>(new FrequencySpectrum(window_function, display_name));

    // Create FFTW buffers, making a separate input buffer if and only if we're using a non-identity input transform.
    const FFTWBuffers context(
            sample_count,
            window_function == FrequencySpectrum::WindowFunction::Identity ? std::make_optional(signal.amplitudes())
                                                                           : std::nullopt
    );

    // Pre-process input as advised by the window function.
    const auto scale_divisor = prepare_input(context, window_function, signal.amplitudes());

    // Create a plan, do the FFT, and clean up. (Buffers are RAII scoped to this function.)
    const fftwf_plan plan = // NOLINT(*-misplaced-const) - Declaration is correct.
            fftwf_plan_dft_r2c_1d(static_cast<int>(sample_count), context.input, context.coefficients, FFTW_ESTIMATE);

    if (plan == nullptr)
        throw std::runtime_error("Failed to create an FFTW plan.");

    fftwf_execute(plan);
    fftwf_destroy_plan(plan);

    // Construct the FrequencySpectrum from the coefficients.
    const auto bin_count = sample_count / 2 + 1;
    auto spectrum = std::unique_ptr<FrequencySpectrum>(new FrequencySpectrum(window_function, display_name));
    spectrum->reserve_bins(bin_count);

    for (std::size_t bin_idx = 0; bin_idx < bin_count; ++bin_idx) {
        const auto is_dc = bin_idx == 0;
        const auto is_nyquist = sample_count % 2 == 0 && bin_idx == sample_count / 2;
        const auto scale = (is_dc || is_nyquist ? 1.0f : 2.0f) / scale_divisor;

        const auto real = context.coefficients[bin_idx][0];
        const auto imag = context.coefficients[bin_idx][1];

        spectrum->emplace_bin(
                static_cast<float>(bin_idx) * static_cast<float>(signal.get_sample_rate()) /
                        static_cast<float>(sample_count),
                std::sqrt(real * real + imag * imag) * scale,
                std::atan2(imag, real)
        );
    }

    return spectrum;
}

float FrequencySpectrumFactory::prepare_input(
        const FFTWBuffers& buffers,
        const FrequencySpectrum::WindowFunction window_function,
        const std::span<const Signal::Sample::AmplitudeT> input
)
{
    assert(input.size() == buffers.input_size);

    switch (window_function) {
    case FrequencySpectrum::WindowFunction::Identity:
        return static_cast<float>(buffers.input_size);

    case FrequencySpectrum::WindowFunction::Hann: {
        float scale_divisor = 0.0f;
        for (std::size_t sample_idx = 0; sample_idx < buffers.input_size; ++sample_idx) {
            const auto window = hann_window(sample_idx, buffers.input_size);
            scale_divisor += window;
            buffers.input[sample_idx] = input[sample_idx] * window;
        }

        return scale_divisor;
    }

    case FrequencySpectrum::WindowFunction::Hamming: {
        float scale_divisor = 0.0f;
        for (std::size_t sample_idx = 0; sample_idx < buffers.input_size; ++sample_idx) {
            const auto window = hamming_window(sample_idx, buffers.input_size);
            scale_divisor += window;
            buffers.input[sample_idx] = input[sample_idx] * window;
        }

        return scale_divisor;
    }
    }

    std::unreachable();
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
        const std::size_t index,
        const std::size_t size
) noexcept
{
    if (size <= 1)
        return 1.0f;

    constexpr float hamming = 25.0f / 46;

    return hamming - (1.0f - hamming) * std::cos(
                                                2.0f * std::numbers::pi_v<float> * static_cast<float>(index) /
                                                static_cast<float>(size - 1)
                                        );
}

} // namespace echomap

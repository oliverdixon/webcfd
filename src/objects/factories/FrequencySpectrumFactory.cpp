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
        const WindowFunctions::AllFunctions window_function,
        const std::size_t transform_size
)
{
    if (!signal.is_uniformly_sampled())
        throw std::runtime_error(std::format("Attempted to transform variably sampled {}.", signal.get_name()));

    const auto display_name = std::format(
            "{} ({} DFT @ {})",
            signal.get_name(),
            WindowFunctions::get_window_name<WindowFunctions::Constant>(),
            transform_size
    );

    if (transform_size == 0)
        return std::unique_ptr<FrequencySpectrum>(new FrequencySpectrum(window_function, display_name));

    // Create FFTW buffers, making a separate input buffer if and only if we're using a non-constant input transform.
    const FFTWBuffers context(
            transform_size,
            std::holds_alternative<WindowFunctions::Constant>(window_function) ? std::make_optional(signal.amplitudes())
                                                                               : std::nullopt
    );

    // Pre-process input as advised by the window function.
    const auto scale_divisor = prepare_input(context, window_function, signal.amplitudes());

    // Create a plan, do the FFT, and clean up. (Buffers are RAII scoped to this function.)
    const fftwf_plan plan = // NOLINT(*-misplaced-const) - Declaration is correct.
            fftwf_plan_dft_r2c_1d(static_cast<int>(transform_size), context.input, context.coefficients, FFTW_ESTIMATE);

    if (plan == nullptr)
        throw std::runtime_error("Failed to create an FFTW plan.");

    fftwf_execute(plan);
    fftwf_destroy_plan(plan);

    // Construct the FrequencySpectrum from the coefficients.
    const auto bin_count = transform_size / 2 + 1;
    auto spectrum = std::unique_ptr<FrequencySpectrum>(new FrequencySpectrum(window_function, display_name));
    spectrum->reserve_bins(bin_count);

    for (std::size_t bin_idx = 0; bin_idx < bin_count; ++bin_idx) {
        const auto is_dc = bin_idx == 0;
        const auto is_nyquist = transform_size % 2 == 0 && bin_idx == transform_size / 2;
        const auto scale = (is_dc || is_nyquist ? 1.0f : 2.0f) / scale_divisor;

        const auto real = context.coefficients[bin_idx][0];
        const auto imag = context.coefficients[bin_idx][1];
        const auto linear_amplitude = std::sqrt(real * real + imag * imag) * scale;

        spectrum->emplace_bin(
                static_cast<float>(bin_idx) * static_cast<float>(signal.get_sample_rate()) /
                        static_cast<float>(transform_size),
                amplitude_to_dbfs(linear_amplitude),
                std::atan2(imag, real)
        );
    }

    return spectrum;
}

float FrequencySpectrumFactory::prepare_input(
        const FFTWBuffers& buffers,
        const WindowFunctions::AllFunctions window_function,
        const std::span<const Signal::Sample::AmplitudeT> input
)
{
    assert(input.size() >= buffers.input_size);

    return std::visit(
            [&buffers, &input]<WindowFunction WindowT>(WindowT) {
                return WindowFunctions::apply_window<WindowT>(buffers, input);
            },
            window_function
    );
}

Signal::Sample::AmplitudeT FrequencySpectrumFactory::amplitude_to_dbfs(
        const Signal::Sample::AmplitudeT amplitude
) noexcept
{
    constexpr auto full = std::max(std::abs(Signal::normalised_range.first), std::abs(Signal::normalised_range.second));
    static_assert(full > 0.0f);

    constexpr auto maximum_ratio = 1.0e-6f; // 20log_{10}(1e-6) = -120dB.
    return 20.0f * std::log10(std::max(std::abs(amplitude) / full, maximum_ratio));
}

} // namespace echomap

/**
 * @file
 *
 * FrequencySpectrumFactory specification
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#ifndef ECHOMAP_FREQUENCYSPECTRUMFACTORY_HPP
#define ECHOMAP_FREQUENCYSPECTRUMFACTORY_HPP

#include <memory>
#include <span>

#include "../FrequencySpectrum.hpp"
#include "../Signal.hpp"

namespace echomap
{

class FFTWBuffers;

/**
 * Provides a static helper to perform discrete Fourier transforms on Signal objects to produce frequency spectra.
 */
class FrequencySpectrumFactory
{
public:
    /**
     * Construct a FrequencySpectrum of the given uniformly sampled Signal.
     *
     * @param signal The uniformly sampled Signal of which to take the DFT.
     * @param window_function The window function to apply onto the input time-series.
     *
     * @return An owning container of the created FrequencySpectrum.
     *
     * @throws std::runtime_error The given Signal was not uniformly sampled; hence, a DFT is not applicable.
     * @throws std::runtime_error FFTW failed to initialise.
     */
    [[nodiscard]] static std::unique_ptr<FrequencySpectrum> create_frequency_spectrum(
            const Signal& signal,
            FrequencySpectrum::WindowFunction window_function
    );

private:
    /**
     * Copies and prepare the input amplitude time-series for FFT with the given window function preference.
     *
     * @param buffers Initialised FFTWBuffers for the input.
     * @param window_function Selector for the window function to use.
     * @param input The amplitude series.
     *
     * @return The scaling divisor constant used for computing the magnitude.
     *
     * @pre The size of the given input range matches the transform size of the FFTWBuffers object.
     */
    static float prepare_input(
            const FFTWBuffers& buffers,
            FrequencySpectrum::WindowFunction window_function,
            std::span<const Signal::Sample::AmplitudeT> input
    );

    /**
     * Provide the Hann window coefficient for the sample at the given index.
     *
     * @param index Index of the sample within the series.
     * @param size Number of samples in the series.
     *
     * @return The Hann coefficient at the index.
     */
    static float hann_window(
            std::size_t index,
            std::size_t size
    ) noexcept;

    /**
     * Provide the Hamming window coefficient for the sample at the given index.
     *
     * @param index Index of the sample within the series.
     * @param size Number of samples in the series.
     *
     * @return The Hamming coefficient at the index.
     */
    static float hamming_window(
            std::size_t index,
            std::size_t size
    ) noexcept;
};

} // namespace echomap

#endif // ECHOMAP_FREQUENCYSPECTRUMFACTORY_HPP

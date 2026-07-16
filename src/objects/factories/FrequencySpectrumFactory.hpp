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
     * @param transform_size The number of samples in the transform window.
     *
     * @return An owning container of the created FrequencySpectrum.
     *
     * @throws std::runtime_error The given Signal was not uniformly sampled; hence, a DFT is not applicable.
     * @throws std::runtime_error FFTW failed to initialise.
     */
    [[nodiscard]] static std::unique_ptr<FrequencySpectrum> create_frequency_spectrum(
            const Signal& signal,
            WindowFunctions::AllFunctions window_function,
            std::size_t transform_size
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
     * @pre The size of the given input range is at least the transform size specified by the FFTWBuffers object.
     */
    static float prepare_input(
            const FFTWBuffers& buffers,
            WindowFunctions::AllFunctions window_function,
            std::span<const Signal::Sample::AmplitudeT> input
    );

    /**
     * Convert an amplitude to a dBfs (decibels relative to full-scale) quantity.
     *
     * @param amplitude The linear amplitude.
     * @return The dBfs quantisation of the linear amplitude.
     */
    [[nodiscard]] static Signal::Sample::AmplitudeT amplitude_to_dbfs(Signal::Sample::AmplitudeT amplitude) noexcept;
};

} // namespace echomap

#endif // ECHOMAP_FREQUENCYSPECTRUMFACTORY_HPP

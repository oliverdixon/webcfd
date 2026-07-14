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

namespace echomap
{

class FrequencySpectrum;
class Signal;

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
     * @return An owning container of the created FrequencySpectrum.
     *
     * @throws std::runtime_error The given Signal was not uniformly sampled; hence, a DFT is not applicable.
     * @throws std::runtime_error FFTW failed to initialise.
     */
    [[nodiscard]] static std::unique_ptr<FrequencySpectrum> create_frequency_spectrum(const Signal& signal);
};

} // namespace echomap

#endif // ECHOMAP_FREQUENCYSPECTRUMFACTORY_HPP

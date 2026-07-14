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
    [[nodiscard]] static std::unique_ptr<FrequencySpectrum> create_frequency_spectrum(const Signal& signal);

private:

};

} // namespace echomap

#endif // ECHOMAP_FREQUENCYSPECTRUMFACTORY_HPP

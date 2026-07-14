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

namespace echomap
{

/**
 * A Signal sampled in the frequency domain.
 */
class FrequencySpectrum : public Object<FrequencySpectrum>
{
public:
    struct Bin
    {
        float frequency;
        float magnitude;
        float phase;
    };

    std::vector<Bin> bins;
};

} // namespace echomap

#endif // ECHOMAP_FREQUENCYSPECTRUM_HPP

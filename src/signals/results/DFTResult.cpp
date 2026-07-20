/**
 * @file
 *
 * DFTResult implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#include "DFTResult.hpp"

namespace echomap
{

DFTResult::DFTResult(
        const id_type source_id,
        std::unique_ptr<FrequencySpectrum> spectrum,
        const std::size_t transform_size
) :
    source_id(source_id),
    spectrum(std::move(spectrum)),
    transform_size(transform_size)
{
    assert(this->spectrum != nullptr);
}

std::unique_ptr<FrequencySpectrum> DFTResult::take_spectrum() && noexcept
{
    return std::move(spectrum);
}

const FrequencySpectrum* DFTResult::observe_spectrum() const noexcept
{
    return spectrum.get();
}

id_type DFTResult::get_source_id() const noexcept
{
    return source_id;
}

std::size_t DFTResult::get_transform_size() const noexcept
{
    return transform_size;
}

} // namespace echomap

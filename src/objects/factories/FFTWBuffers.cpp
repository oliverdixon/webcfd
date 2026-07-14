/**
 * @file
 *
 * FFTWBuffers implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#include <cassert>
#include <stdexcept>

#include "FFTWBuffers.hpp"

namespace echomap
{

FFTWBuffers::FFTWBuffers(
        const std::size_t size,
        const std::optional<std::span<const float>>& inplace_input
) :
    coefficients(fftwf_alloc_complex(size / 2 + 1)),
    input(inplace_input.has_value() ? const_cast<float*>(inplace_input->data()) : fftwf_alloc_real(size)),
    input_size(size),
    is_input_managed(!inplace_input.has_value())
{
    /*
     * It's not very nice, but we cast away the const on the input buffer as required by the FFTW C API. We know that
     * in-place transforms are not going to take place, so it doesn't make sense to take optional<float*> (mutable
     * pointer) as a constructor parameter.
     */

    if (coefficients == nullptr)
        throw std::runtime_error("Failed to create FFTW coefficients buffer.");

    if (!inplace_input.has_value() && input == nullptr) {
        fftwf_free(coefficients);
        throw std::runtime_error("Failed to create FFTW input buffer.");
    }

    assert(input != nullptr);
    assert(coefficients != nullptr);
}

FFTWBuffers::~FFTWBuffers() noexcept
{
    if (is_input_managed && input != nullptr)
        fftwf_free(input);

    if (coefficients != nullptr)
        fftwf_free(coefficients);
}

} // namespace echomap

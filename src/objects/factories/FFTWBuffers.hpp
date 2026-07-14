/**
 * @file
 *
 * FFTWBuffers specification
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#ifndef ECHOMAP_FFTWBUFFERS_HPP
#define ECHOMAP_FFTWBUFFERS_HPP

#include <fftw3.h>

#include <cstddef>
#include <optional>
#include <span>

namespace echomap
{

/**
 * Simple RAII wrapper for commonly used C types from FFTW(f).
 */
class FFTWBuffers
{
public:
    fftwf_complex* const coefficients = nullptr; /**< Complex coefficients (output of real-to-complex FFT). */
    float* const input = nullptr; /**< Real-valued inputs, either managed by us or aliased by the user. */

    /**
     * Setup contextual buffers for FFTW.
     *
     * @param size The number of samples in the input data.
     * @param inplace_input The array to which the input should be aliased; if not provided, FFTW allocates a
     *  real-valued array in the size of the given sample count.
     *
     * @throws std::runtime_error One or more buffers could not be allocated.
     *
     * @post The coefficients buffer is non-null.
     * @post The input buffer is non-null.
     */
    explicit FFTWBuffers(
            std::size_t size,
            const std::optional<std::span<const float>>& inplace_input
    );

    /**
     * Deallocate the buffers.
     */
    ~FFTWBuffers() noexcept;

private:
    const bool is_input_managed; /**< Are we responsible for the input buffer? */
};

} // namespace echomap

#endif // ECHOMAP_FFTWBUFFERS_HPP

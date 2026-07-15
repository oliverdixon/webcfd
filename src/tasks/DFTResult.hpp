/**
 * @file
 *
 * DFTResult specification
 *
 * @author Oliver Dixon
 * @date 2026-07-14
 */

#ifndef ECHOMAP_DFTRESULT_HPP
#define ECHOMAP_DFTRESULT_HPP

#include "../objects/FrequencySpectrum.hpp"
#include "../objects/Signal.hpp"

namespace echomap
{

/**
 * Denotes a completed DFT computation from a DFTTask.
 */
class DFTResult
{
public:
    explicit DFTResult(
            Signal::id_type source_id,
            std::unique_ptr<FrequencySpectrum> spectrum,
            std::size_t transform_size
    );

    [[nodiscard]] std::unique_ptr<FrequencySpectrum> take_spectrum() && noexcept;
    [[nodiscard]] const FrequencySpectrum* observe_spectrum() const noexcept;
    [[nodiscard]] Signal::id_type get_source_id() const noexcept;
    [[nodiscard]] std::size_t get_transform_size() const noexcept;

private:
    const Signal::id_type source_id;
    std::unique_ptr<FrequencySpectrum> spectrum;
    const std::size_t transform_size;
};

} // namespace echomap

#endif // ECHOMAP_DFTRESULT_HPP

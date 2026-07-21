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

#include <memory>

#include "../../objects/IDAllocator.hpp"

namespace echomap
{

class FrequencySpectrum;
class Signal;

/**
 * Denotes a completed DFT computation from a DFTTask.
 */
class DFTResult
{
public:
    explicit DFTResult(
            id_type source_id,
            std::unique_ptr<FrequencySpectrum> spectrum,
            std::size_t transform_size
    );

    ~DFTResult() noexcept;

    DFTResult(const DFTResult&) = delete;
    DFTResult& operator=(const DFTResult&) = delete;

    DFTResult(DFTResult&&) noexcept;
    DFTResult& operator=(DFTResult&&) noexcept;

    [[nodiscard]] std::unique_ptr<FrequencySpectrum> take_spectrum() && noexcept;
    [[nodiscard]] const FrequencySpectrum* observe_spectrum() const noexcept;
    [[nodiscard]] id_type get_source_id() const noexcept;
    [[nodiscard]] std::size_t get_transform_size() const noexcept;

private:
    id_type source_id;
    std::unique_ptr<FrequencySpectrum> spectrum;
    std::size_t transform_size;
};

} // namespace echomap

#endif // ECHOMAP_DFTRESULT_HPP

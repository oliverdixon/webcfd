/**
 * @file
 *
 * IDAllocator specification
 *
 * @author Oliver Dixon
 * @date 2026-06-25
 */

#ifndef ECHOMAP_IDALLOCATOR_HPP
#define ECHOMAP_IDALLOCATOR_HPP

#include <atomic>
#include <cassert>
#include <cstdint>
#include <limits>

namespace echomap
{

using id_type = std::uint64_t;

/**
 * Provides a simple thread-safe runtime ID generator for Object managers.
 */
template<typename>
class IDAllocator
{
public:
    /**
     * Sentinel ID used by invalidated (moved-from) objects.
     *
     * This value is reserved and must never be returned by @ref allocate.
     */
    static constexpr id_type invalid_id = std::numeric_limits<id_type>::max();

    /**
     * Allocate the next ID.
     *
     * @return The next ID.
     * @pre The datatype will be sufficient to store the subsequent ID following allocation.
     */
    [[nodiscard]] static id_type allocate() noexcept
    {
        const auto allocated = next_id.fetch_add(1, std::memory_order_relaxed);
        assert(allocated != invalid_id);
        return allocated;
    }

private:
    inline static std::atomic<id_type> next_id = 0;
};

} // namespace echomap

#endif // ECHOMAP_IDALLOCATOR_HPP

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

#include <cassert>
#include <cstdint>
#include <limits>

namespace echomap
{

using id_type = std::uint64_t;

/**
 * Provides a simple static runtime ID generator for Object managers.
 */
template<typename>
class IDAllocator
{
public:
    /**
     * Allocate the next ID.
     *
     * @return The next ID.
     * @pre The datatype will be sufficient to store the subsequent ID following allocation.
     */
    [[nodiscard]] static id_type allocate() noexcept
    {
        assert(next_id < std::numeric_limits<id_type>::max());
        return next_id++;
    }

private:
    static id_type next_id;
};

template<typename Class>
id_type IDAllocator<Class>::next_id = 0;

} // namespace echomap

#endif // ECHOMAP_IDALLOCATOR_HPP

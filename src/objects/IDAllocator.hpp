//
// Created by owd on 25/06/2026.
//

#ifndef ECHOMAP_IDALLOCATOR_HPP
#define ECHOMAP_IDALLOCATOR_HPP

#include <cstdint>

namespace echomap
{

template<typename>
class IDAllocator
{
public:
    using id_type = std::uint64_t;

    [[nodiscard]] static id_type allocate() noexcept
    {
        return next_id++;
    }

private:
    static id_type next_id;
};

template<typename Class>
IDAllocator<Class>::id_type IDAllocator<Class>::next_id = 0;

} // namespace echomap

#endif // ECHOMAP_IDALLOCATOR_HPP

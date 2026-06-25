//
// Created by owd on 25/06/2026.
//

#ifndef WEBCFD_OBJECT_HPP
#define WEBCFD_OBJECT_HPP

#include <string>
#include <string_view>

#include "IDAllocator.hpp"

namespace WebCFD
{

template <typename Derived> class Object
{
public:
    using id_type = IDAllocator<Derived>::id_type;

    [[nodiscard]] id_type get_id() const noexcept
    {
        return id;
    }

    void set_name(
            const std::string_view new_name
    )
    {
        this->name = std::string(new_name);
    }

    [[nodiscard]] std::string_view get_name() const noexcept
    {
        return name;
    }

    [[nodiscard]] const char* get_imgui_name() const noexcept
    {
        return name.c_str();
    }

    [[nodiscard]] static std::string_view get_class_name() noexcept
    {
        return class_name;
    }

protected:
    Object() :
        id(IDAllocator<Derived>::allocate()),
        name(class_name + ' ' + std::to_string(id))
    {
    }

    explicit Object(
            const std::string_view object_name
    ) :
        id(IDAllocator<Derived>::allocate()),
        name(object_name.empty() ? class_name + ' ' + std::to_string(id) : object_name)
    {
    }

private:
    /**
     * Display name for objects of the type determined by the templated class.
     *
     * <p>
     *  Inheriting classes should override the class name by doing something equivalent to:
     *  <pre>
     *      template <>
     *      constexpr std::string Object<Sensor>::class_name = "Sensor";
     *  </pre>
     * </p>
     * <p>
     *  The current implementation is relying on non-standard std::string small-string library optimisations to place
     *  string literals of 15 (16 inc. NULL) on the stack. Anything longer will perform heap allocation and hence
     *  violate constexpr requirements. Should this be revised? Probably. Will it be? Probably not.
     * </p>
     */
    static constexpr std::string class_name = "Untyped Object";

    const id_type id;
    std::string name;
};

} // namespace WebCFD

#endif // WEBCFD_OBJECT_HPP

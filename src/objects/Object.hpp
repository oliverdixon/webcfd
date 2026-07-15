/**
 * @file
 *
 * Object specification
 *
 * @author Oliver Dixon
 * @date 2026-06-25
 */

#ifndef ECHOMAP_OBJECT_HPP
#define ECHOMAP_OBJECT_HPP

#include <format>
#include <string>
#include <string_view>

#include "IDAllocator.hpp"

namespace echomap
{

/**
 * A participant in a runtime object model with a numerically unique identifier and display name.
 *
 * <p>
 *  A design invariant, which is not realistically possible to enforce, requires that no two objects of the same class
 *  co-exist for any non-trivial duration with matching IDs. In this context, a "non-trivial lifetime" would be any
 *  state that outlives a function as a member or global variable.
 * </p>
 * <p>
 *  In general, users should refer to objects by their stable numerical ID as opposed to iterators, pointers, or
 *  references. Transparent hashing on ordered and unordered STL containers allows fast and convenient lookup in the ID.
 * </p>
 *
 * @tparam Derived The concrete derived type, used for CRTP static polymorphism.
 */
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

    // ReSharper disable once CppDFAConstantFunctionResult
    [[nodiscard]] static std::string_view get_class_name() noexcept
    {
        return class_name;
    }

    /**
     * Deleted copy constructor.
     *
     * Derived classes should provide their own copy constructors to enable copy behaviour.
     */
    Object(const Object&) = delete;

    /**
     * Deleted copy-assignment operator.
     *
     * @see Object(const Object&).
     */
    Object& operator=(const Object&) = delete;

    /**
     * Defaulted move constructor.
     *
     * Move operations do not allocate a new internal ID.
     */
    Object(Object&&) = default;

    /**
     * Defaulted move-assignment operator.
     *
     * @return Mutable reference to the moved Object.
     */
    Object& operator=(Object&&) = default;

    /**
     * Determine shallow equality between two Object instances.
     *
     * Equality is solely determined by the primary ID.
     *
     * @param other The Object to compare against.
     * @return Do the IDs indicate that the current Object is equal to the other one?
     */
    [[nodiscard]] bool operator==(
            const Object& other
    ) const noexcept
    {
        return id == other.id;
    }

protected:
    /**
     * Tag despatch discriminator for copying the base object, preserving uniqueness of IDs.
     */
    struct CopyTag
    {};

    /**
     * Create a new Object with an auto-allocated ID and default display name.
     */
    Object() :
        id(IDAllocator<Derived>::allocate()),
        name(std::string(class_name) + ' ' + std::to_string(id))
    {
    }

    /**
     * Create a new Object with an auto-allocated ID and custom display name.
     *
     * @param object_name The display name of the object. If empty, uses a sensible default based on the ID.
     */
    explicit Object(
            const std::string_view object_name
    ) :
        id(IDAllocator<Derived>::allocate()),
        name(object_name.empty() ? std::string(class_name) + ' ' + std::to_string(id) : object_name)
    {
    }

    /**
     * Copy an existing Object.
     *
     * The new Object receives a newly allocated ID and a sensible default display name.
     *
     * @param old The existing Object to copy.
     */
    Object(
            CopyTag,
            const Object& old
    ) :
        id(IDAllocator<Derived>::allocate()),
        copy_count(old.copy_count + 1),
        name(std::format(
                "{} ({})",
                old.get_name(),
                copy_count
        ))
    {
    }

    /**
     * Copy an existing Object.
     *
     * The new Object receives a newly allocated ID.
     *
     * @param old The existing Object to copy.
     * @param new_name The display name of the new Object.
     */
    Object(
            CopyTag,
            const Object& old,
            const std::string_view new_name
    ) :
        id(IDAllocator<Derived>::allocate()),
        copy_count(old.copy_count),
        name(new_name)
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
     *      constexpr std::string_view Object<Sensor>::class_name = "Sensor";
     *  </pre>
     * </p>
     */
    static constexpr std::string_view class_name = "Object";

    const id_type id;           /**< Primary numerical ID */
    std::size_t copy_count = 0; /**< Number of times the object has been copied */
    std::string name;           /**< Display name for the object */
};

} // namespace echomap

#endif // ECHOMAP_OBJECT_HPP

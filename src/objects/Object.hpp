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
#include <utility>

#include "IDAllocator.hpp"

namespace echomap
{

/**
 * A participant in a runtime object model with a numerically unique identifier and display name.
 *
 * A design invariant, which is not realistically possible to enforce, requires that no two objects of the same class
 * co-exist for any non-trivial duration with matching IDs. In this context, a "non-trivial lifetime" would be any
 * state that outlives a function as a member or global variable.
 *
 * In general, users should refer to objects by their stable numerical ID as opposed to iterators, pointers, or
 * references. Transparent hashing on ordered and unordered STL containers allows fast and convenient lookup in the ID.
 *
 * @tparam Derived The concrete derived type, used for CRTP static polymorphism.
 */
template <typename Derived> class Object
{
public:
    /**
     * Non-virtual base destructor.
     *
     * We don't want vtables for Object instances. Object uses CRTP, so deleting an Object through @c Object* is not a
     * concern.
     */
    ~Object() = default;

    [[nodiscard]] id_type get_id() const noexcept
    {
        assert(is_valid());
        return id;
    }

    [[nodiscard]] bool is_valid() const noexcept
    {
        return id != IDAllocator<Derived>::invalid_id;
    }

    void set_name(
            const std::string_view new_name
    )
    {
        assert(is_valid());
        this->name = std::string(new_name);
    }

    [[nodiscard]] std::string_view get_name() const noexcept
    {
        assert(is_valid());
        return name;
    }

    [[nodiscard]] const char* get_imgui_name() const noexcept
    {
        assert(is_valid());
        return name.c_str();
    }

    [[nodiscard]] static std::string_view get_class_name() noexcept
    {
        return class_name;
    }

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    Object& operator=(Object&&) = delete;

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

    Object(
            Object&& other
    ) noexcept :
        id(std::exchange(
                other.id,
                IDAllocator<Derived>::invalid_id
        )),
        copy_count(
                std::exchange(
                        other.copy_count,
                        0
                )
        ),
        name(std::move(other.name))
    {
    }

    /**
     * Copy an existing Object.
     *
     * The new Object receives a newly allocated ID and a sensible default display name.
     *
     * @param tag CopyTag despatch tag.
     * @param old The existing Object to copy.
     */
    Object(
            [[maybe_unused]] CopyTag tag,
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
        ++old.copy_count;
    }

    /**
     * Copy an existing Object.
     *
     * The new Object receives a newly allocated ID.
     *
     * @param tag CopyTag despatch tag.
     * @param old The existing Object to copy.
     * @param new_name The display name of the new Object.
     */
    Object(
            [[maybe_unused]] CopyTag tag,
            const Object& old,
            const std::string_view new_name
    ) :
        id(IDAllocator<Derived>::allocate()),
        copy_count(old.copy_count),
        name(new_name)
    {
    }

    /**
     * Helper for derived classes to provide move-assignment operations.
     *
     * For example,
     * @code
     * Sensor& operator=(Sensor&& other) noexcept
     * {
     *     if (this == &other)
     *         return *this;
     *
     *     Object::move_identity_from(std::move(other));
     *
     *     position = std::move(other.position);
     *     colour = std::move(other.colour);
     *
     *     return *this;
     * }
     * @endcode
     *
     * @param other The Object being moved from, to have its state invalidated.
     */
    void move_identity_from(
            Object&& other // NOLINT(*-rvalue-reference-param-not-moved)
    ) noexcept
    {
        if (this == &other)
            return;

        assert(other.is_valid());

        id = std::exchange(other.id, IDAllocator<Derived>::invalid_id);
        copy_count = std::exchange(other.copy_count, 0);
        name = std::move(other.name);
    }

private:
    /**
     * Display name for objects of the type determined by the templated class.
     *
     * Inheriting classes should override the class name by doing something equivalent to:
     * @code
     * template <>
     * constexpr std::string_view Object<Sensor>::class_name = "Sensor";
     * @endcode
     */
    static constexpr std::string_view class_name = "Object";

    id_type id;                         /**< Primary numerical ID. */
    mutable std::size_t copy_count = 0; /**< Number of times the object has been copied; used only for display names. */
    std::string name;                   /**< Display name for the object. */
};

} // namespace echomap

#endif // ECHOMAP_OBJECT_HPP

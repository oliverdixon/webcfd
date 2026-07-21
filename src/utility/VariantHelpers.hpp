/**
 * @file
 *
 * VariantHelpers specification
 *
 * @author Oliver Dixon
 * @date 2026-07-17
 */

#ifndef ECHOMAP_VARIANTHELPERS_HPP
#define ECHOMAP_VARIANTHELPERS_HPP

#include <stdexcept>
#include <utility>
#include <variant>

namespace echomap::variant_helpers
{

/**
 * A helper type for succinctly specifying callable visitors over a variant alternative.
 *
 * For example,
 *
 * @code
 * std::variant<A, B, C> v;
 *
 * // populate v.
 *
 * std::visit(Overloaded{
 *     [](const A&) { handle_a(a); },
 *     [](const B&) { handle_b(b); },
 *     [](const C&) { handle_c(c); },
 * }, v);
 * @endcode
 *
 * Taken from https://cppreference.com/cpp/utility/variant/visit2.
 *
 * @tparam Ts Types of callables; deduced implicitly since C++20.
 */
template <class... Ts> struct Overloaded : Ts...
{
    using Ts::operator()...;
};

/**
 * Produces an array of human-readable names for selected alternatives of a variant.
 *
 * Given a variant type @c std::variant<A, B, C>, a name getter @c NameGetter, and an index sequence equivalent to
 * @c std::index_sequence<0, 1, 2>, this function returns an array containing:
 *
 * @code
 * NameGetter::get<A>(),
 * NameGetter::get<B>(),
 * NameGetter::get<C>()
 * @endcode
 *
 * The returned array preserves the alternative ordering of @p Variant. Therefore, the name at position @f$ i @f$
 * corresponds to the @f$ i @f$-th selected alternative index in the supplied index sequence.
 *
 * @tparam Variant The variant type whose alternative names are generated.
 * @tparam NameGetter A policy type providing @c NameGetter::template @c get<T>() for each selected alternative type.
 * @tparam indices The alternative indices for which names are generated.
 *
 * @param sequence The index sequence identifying the variant alternatives. The value is used only for template argument
 *  deduction.
 *
 * @return An array of human-readable names corresponding to the selected alternatives.
 *
 * @pre @p Variant must be a specialization of @c std::variant.
 * @pre Each value in @p Indices must be a valid alternative index for @p Variant.
 * @pre @p NameGetter must provide a valid @c get<T>() function for every selected alternative type.
 */
template <
        class Variant,
        class NameGetter,
        std::size_t... indices>
consteval auto make_variant_name_array(
        std::index_sequence<indices...> sequence
)
{
    std::ignore = sequence;
    return std::array<std::string_view, sizeof...(indices)>{
            NameGetter::template get<std::variant_alternative_t<indices, Variant>>()...
    };
}

/**
 * Array of human-readable names for every alternative in a variant.
 *
 * The array order matches the alternative order of @p Variant. Therefore, index @f$ i @f$ in this array names the same
 * alternative selected by @c std::variant::index() == i.
 *
 * Names are obtained from @p NameGetter, which must provide a templated static function of the form:
 *
 * @code
 * template<class T>
 * static consteval std::string_view get();
 * @endcode
 *
 * @tparam Variant The variant type whose alternatives are named.
 * @tparam NameGetter A policy type used to obtain names for each alternative type.
 *
 * @pre @p Variant must be a specialization of @c std::variant.
 * @pre Every alternative type in @p Variant must be accepted by @p NameGetter.
 */
template <class Variant, class NameGetter>
inline constexpr auto variant_name_array =
        make_variant_name_array<Variant, NameGetter>(std::make_index_sequence<std::variant_size_v<Variant>>{});

/**
 * Constructs a variant holding the alternative at a compile-time index.
 *
 * The returned variant is constructed with @c std::in_place_index for @p Index. Therefore, the initially held
 * alternative is the default-constructed @p Index-th alternative type of @p Variant.
 *
 * For example, for @c std::variant<A, B, C>, calling this function with @p Index set to @c 1 returns a variant holding
 * a default-constructed @c B.
 *
 * @tparam Variant The variant type to construct.
 * @tparam index The index of the alternative to hold initially.
 *
 * @return A @p Variant holding a default-constructed instance of the alternative at @p Index.
 *
 * @pre @p Variant must be a specialization of @c std::variant.
 * @pre @p Index must be a valid alternative index for @p Variant.
 * @pre The selected alternative type must be default-constructible.
 */
template <
        class Variant,
        std::size_t index>
constexpr Variant make_variant_alternative()
{
    return Variant{std::in_place_index<index>};
}

/**
 * Produces an array of factory function pointers for constructing a variant by alternative index.
 *
 * Given a variant type @c std::variant<A, B, C> and an index sequence equivalent to @c std::index_sequence<0, 1, 2>,
 * this function returns an array of type:

 * @code std::array<std::variant<A, B, C> (*)(), 3> @endcode
 *
 * The function pointer at position @f$ i @f$ constructs a @p Variant whose initially held alternative is the
 * default-constructed @f$ i @f$-th alternative type.
 *
 * For example, calling the factory at index @c 1 for @c std::variant<A, B, C> returns a variant holding a
 * default-constructed @c B.
 *
 * @tparam Variant The variant type constructed by the returned factory functions.
 * @tparam indices The alternative indices for which factory functions are generated.
 *
 * @param sequence The index sequence identifying the variant alternatives. The value is used only for template argument
 *  deduction.
 *
 * @return An array of function pointers. Each function constructs a @p Variant holding the default-constructed
 *  alternative corresponding to its index.
 *
 * @pre Each value in @p Indices must be a valid alternative index for @p Variant.
 * @pre Each selected alternative type must be default-constructible.
 */
template <
        class Variant,
        std::size_t... indices>
consteval auto make_variant_factory_array(
        std::index_sequence<indices...> sequence
)
{
    std::ignore = sequence;
    using Factory = Variant (*)();

    return std::array<Factory, sizeof...(indices)>{&make_variant_alternative<Variant, indices>...};
}

/**
 * Constructs a @c std::variant holding the alternative at the given runtime index.
 *
 * The returned variant is constructed with @c std::in_place_index for the requested alternative index. The selected
 * alternative is default-constructed.
 *
 * @tparam Variant The variant type to construct.

 * @param index The index of the alternative that should be initially held by the variant.
 *
 * @return A @p Variant holding a default-constructed instance of the alternative at @p index.
 *
 * @throws std::out_of_range @p index is not a valid alternative index for @p Variant.
 *
 * @pre @p Variant must be a specialization of @c std::variant.
 * @pre The selected alternative type must be default-constructible.
 */
template <class Variant>
Variant variant_from_index(
        const std::size_t index
)
{
    static constexpr auto factories =
            make_variant_factory_array<Variant>(std::make_index_sequence<std::variant_size_v<Variant>>{});

    if (index >= factories.size())
        throw std::out_of_range("Invalid variant index.");

    return factories[index]();
}

} // namespace echomap::variant_helpers

#endif // ECHOMAP_VARIANTHELPERS_HPP

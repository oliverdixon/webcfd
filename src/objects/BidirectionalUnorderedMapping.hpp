/**
 * @file
 * @brief WebCFD bidirectional unordered mapping container specification and implementation
 * @author Oliver Dixon
 * @date 2026-06-25
 */

#ifndef WEBCFD_BIDIRECTIONALUNORDEREDMAPPING_HPP
#define WEBCFD_BIDIRECTIONALUNORDEREDMAPPING_HPP

#include <cassert>
#include <unordered_map>

namespace WebCFD
{

/**
 * Maintains a bidirectional mapping between a key and value, allowing fast lookup on both objects.
 *
 * @tparam Key The first key type
 * @tparam Value The second key type, colloquially the "value".
 *
 * <p>
 *  The templated types should typically be small (such as a stable integral ID), since their values are duplicated
 *  internally across two STL unordered hashing containers.
 * </p>
 * <p>
 *  To guarantee implementation invariants, the bidirectional mapping container does not expose mutable access to its
 *  members. Mutation of the iterators of the underlying storage containers can be done exclusively through the public
 *  interface.
 * </p>
 */
template <typename Key, typename Value> class BidirectionalUnorderedMapping
{
    /**
     * The "forward" map, mapping keys to values in the conventional way.
     *
     * @invariant Every K-V entry in the forward map relates to a corresponding V-K entry in the reverse map.
     * @invariant The forward map contains no excess elements than those in the reverse map.
     */
    std::unordered_map<Key, Value> forward_map;

    /**
     * The "reverse" map, mapping values to keys.
     *
     * @invariant Every V-K entry in the reverse map relates to a corresponding K-V entry in the forward map.
     * @invariant The reverse map contains no excess elements than those in the forward map.
     */
    std::unordered_map<Value, Key> reverse_map;

public:
    using iterator = decltype(forward_map)::iterator; /**< The bidirectional container non-constant iterator. */
    using const_iterator = decltype(forward_map)::const_iterator; /**< The bidirectional container constant iterator. */

    [[nodiscard]] const_iterator begin() const noexcept
    {
        return forward_map.begin();
    }

    [[nodiscard]] const_iterator cbegin() const noexcept
    {
        return forward_map.cbegin();
    }

    [[nodiscard]] const_iterator end() const noexcept
    {
        return forward_map.end();
    }

    [[nodiscard]] const_iterator cend() const noexcept
    {
        return forward_map.cend();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return forward_map.empty();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return forward_map.size();
    }

    /**
     * Removes all entries from the mapping.
     *
     * @post All internal containers are cleared.
     */
    void clear() noexcept
    {
        forward_map.clear();
        reverse_map.clear();

        assert(forward_map.empty() == reverse_map.empty() == true);
    }

    /**
     * Emplaces an entry to the mapping.
     *
     * @tparam KeyT Type of the forward key
     * @tparam ValueT Type of the forward value
     * @param key Key from the forward perspective
     * @param value Value from the forward perspective
     * @return
     *  <ol>
     *      <li>Constant iterator to the inserted element, or the end iterator if the insertion failed.</li>
     *      <li>Did an insertion take place?</li>
     *  </ol>
     */
    template <
            typename KeyT,
            typename ValueT>
    std::pair<
            const_iterator,
            bool>
    emplace(
            KeyT&& key,
            ValueT&& value
    )
    {
        auto forward_insertion = forward_map.emplace(std::forward<KeyT>(key), std::forward<ValueT>(value));
        if (!forward_insertion.second)
            return {forward_insertion.first, false};

        try {
            const auto& inserted_key = forward_insertion.first->first;
            // ReSharper disable once CppTooWideScopeInitStatement - Intentionally wide for readability.
            const auto& inserted_value = forward_insertion.first->second;

            if (!reverse_map.emplace(inserted_value, inserted_key).second) {
                forward_map.erase(forward_insertion.first);
                return {forward_map.end(), false};
            }
        } catch (...) {
            forward_map.erase(forward_insertion.first);
            throw;
        }

        assert(forward_map.size() == reverse_map.size());
        return {forward_insertion.first, true};
    }

    /**
     * Removes an entry from the mapping by its key.
     *
     * @param key The key of the entry to remove.
     * @return Was an element removed?
     * @post All internal containers meet the size invariant.
     */
    bool erase_by_key(
            const Key& key
    )
    {
        const auto it = forward_map.find(key);
        if (it == forward_map.end())
            return false;

        reverse_map.erase(it->second);
        forward_map.erase(it);

        assert(forward_map.size() == reverse_map.size());

        return true;
    }

    /**
     * Removes an entry from the mapping by its value.
     *
     * @param value The value of the entry to remove.
     * @return Was an element removed?
     * @post All internal containers meet the size invariant.
     */
    bool erase_by_value(
            const Value& value
    )
    {
        const auto it = reverse_map.find(value);
        if (it == reverse_map.end())
            return false;

        forward_map.erase(it->second);
        reverse_map.erase(it);

        assert(forward_map.size() == reverse_map.size());

        return true;
    }

    /**
     * Locates an entry in the mapping by its key.
     *
     * @param key The key of the entry to locate.
     * @return A constant iterator to the located element, or the end iterator of the container if no element was found.
     */
    const_iterator find_by_key(
            const Key& key
    ) const
    {
        return forward_map.find(key);
    }

    /**
     * Locates an entry in the mapping by its value.
     *
     * @param value The value of the entry to locate.
     * @return A constant iterator to the located element, or the end iterator of the container if no element was found.
     */
    const_iterator find_by_value(
            const Value& value
    ) const
    {
        const auto reverse_map_it = reverse_map.find(value);
        if (reverse_map_it == reverse_map.end())
            return forward_map.end();

        const auto forward_map_it = forward_map(reverse_map_it->second);
        assert(forward_map_it != forward_map.end()); // Verify class invariant.

        return forward_map_it;
    }
};

} // namespace WebCFD

#endif // WEBCFD_BIDIRECTIONALUNORDEREDMAPPING_HPP

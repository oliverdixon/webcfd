/**
 * @file
 *
 * ScopedConnections specification
 *
 * @author Oliver Dixon
 * @date 2026-07-15
 */

#ifndef ECHOMAP_SCOPEDCONNECTIONS_HPP
#define ECHOMAP_SCOPEDCONNECTIONS_HPP

#include <sigc++/sigc++.h>

namespace echomap
{

/**
 * Provides a composable RAII wrapper for libsigc++ scoped connections.
 */
class ScopedConnections
{
public:
    ScopedConnections() = default;

    ScopedConnections(const ScopedConnections&) = delete;
    ScopedConnections& operator=(const ScopedConnections&) = delete;

    ScopedConnections(ScopedConnections&&) noexcept = default;
    ScopedConnections& operator=(ScopedConnections&&) noexcept = default;

    void add(sigc::connection connection);

private:
    std::vector<sigc::scoped_connection> connections;
};

} // namespace echomap

#endif // ECHOMAP_SCOPEDCONNECTIONS_HPP

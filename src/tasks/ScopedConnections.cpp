/**
 * @file
 *
 * ScopedConnections implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-15
 */

#include "ScopedConnections.hpp"

namespace echomap
{

void ScopedConnections::add(
        sigc::connection connection
)
{
    connections.emplace_back(std::move(connection));
}

} // namespace echomap

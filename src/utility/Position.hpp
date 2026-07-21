/**
 * @file
 *
 * Position specification
 *
 * @author Oliver Dixon
 * @date 2026-07-21
 */

#ifndef ECHOMAP_POSITION_HPP
#define ECHOMAP_POSITION_HPP

namespace echomap
{

/**
 * A generic 3D position structure.
 */
struct Position
{
    float x = 0.0f; /**< The X co-ordinate. */
    float y = 0.0f; /**< The Y co-ordinate. */
    float z = 0.0f; /**< The Z co-ordinate. */
};

} // namespace echomap

#endif // ECHOMAP_POSITION_HPP

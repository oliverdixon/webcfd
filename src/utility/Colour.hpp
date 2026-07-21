/**
 * @file
 *
 * Colour specification
 *
 * @author Oliver Dixon
 * @date 2026-07-21
 */

#ifndef ECHOMAP_COLOUR_HPP
#define ECHOMAP_COLOUR_HPP

namespace echomap
{

/**
 * A generic RGBA structure.
 */
struct Colour
{
    float r = 1.0f; /**< The red component in the range [0, 1]. */
    float g = 0.0f; /**< The green component in the range [0, 1]. */
    float b = 0.0f; /**< The blue component in the range [0, 1]. */
    float a = 1.0f; /**< The alpha extent in the range [0, 1]. */
};

} // namespace echomap

#endif // ECHOMAP_COLOUR_HPP

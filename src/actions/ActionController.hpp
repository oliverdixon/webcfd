/**
 * @file
 *
 * ActionController specification
 *
 * @author Oliver Dixon
 * @date 2026-07-18
 */

#ifndef ECHOMAP_ACTIONCONTROLLER_HPP
#define ECHOMAP_ACTIONCONTROLLER_HPP

#ifdef __EMSCRIPTEN__
#include "JSActionController.hpp"
#else
#include "NativeActionController.hpp"
#endif

namespace echomap
{

#ifdef __EMSCRIPTEN__
using ActionController = JSActionController;
#else
using ActionController = NativeActionController;
#endif

} // namespace echomap

#endif // ECHOMAP_ACTIONCONTROLLER_HPP

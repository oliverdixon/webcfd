/**
 * @file
 *
 * StaticInstanceController implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-18
 */

#include "StaticInstanceController.hpp"

#include <sigc++/sigc++.h>

#include "EchoMap.hpp"
#include "actions/ActionController.hpp"

namespace echomap
{

StaticInstanceController::StaticInstanceController(
        EchoMap& app
)
{
    ActionController::rebind(sigc::mem_fun(app, &EchoMap::update_project));
}

StaticInstanceController::~StaticInstanceController() noexcept
{
    ActionController::rebind();
}

} // namespace echomap

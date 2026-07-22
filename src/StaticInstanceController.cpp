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
#include "notifications/AllNotifications.hpp"

namespace echomap
{

StaticInstanceController::StaticInstanceController(
        EchoMap& app
)
{
    ActionController::bind([&app](const std::filesystem::path& path) {
        app.notify(ProjectSelectedNotification(path));
    },
            [&app](const std::size_t project_id,
                   const std::filesystem::path& external,
                   const std::filesystem::path& internal) {
                app.notify(RegisterVFSMappingNotification(project_id, external, internal));
            }
    );
}

StaticInstanceController::~StaticInstanceController() noexcept
{
    ActionController::unbind();
}

} // namespace echomap

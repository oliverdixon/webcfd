/**
 * @file
 *
 * ProjectSelectedNotification specification
 *
 * @author Oliver Dixon
 * @date 2026-07-19
 */

#ifndef ECHOMAP_PROJECTSELECTEDNOTIFICATION_HPP
#define ECHOMAP_PROJECTSELECTEDNOTIFICATION_HPP

#include <filesystem>

namespace echomap
{

/**
 * A notification indicating that a new Project file has been selected by the user to load into the application.
 *
 * @ingroup Notifications
 */
struct ProjectSelectedNotification
{
    /**
     * Create a new ProjectSelectedNotification.
     *
     * @param path The path of the Project file to load.
     */
    explicit ProjectSelectedNotification(
            std::filesystem::path path
    ) :
        path(std::move(path))
    {
    }

    std::filesystem::path path; /**< The path of the Project file to load. */
};

} // namespace echomap

#endif // ECHOMAP_PROJECTSELECTEDNOTIFICATION_HPP

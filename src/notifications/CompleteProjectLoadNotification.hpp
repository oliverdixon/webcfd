/**
 * @file
 *
 * CompleteProjectLoadNotification specification
 *
 * @author Oliver Dixon
 * @date 2026-07-19
 */

#ifndef ECHOMAP_COMPLETEPROJECTLOADNOTIFICATION_HPP
#define ECHOMAP_COMPLETEPROJECTLOADNOTIFICATION_HPP

#include "../objects/Project.hpp"

namespace echomap
{

/**
 * A notification indicating that a pending Project is ready to be loaded.
 *
 * @ingroup Notifications
 */
class CompleteProjectLoadNotification
{
public:
    /**
     * Create a CompleteProjectLoadNotification to indicate that the referenced Project is ready to be loaded.
     *
     * @param project_id The ID of the Project to be loaded.
     */
    explicit CompleteProjectLoadNotification(
            const Project::id_type project_id
    ) :
        project_id(project_id)
    {
    }

    Project::id_type project_id; /**< The ID of the Project to be loaded. */
};

} // namespace echomap

#endif // ECHOMAP_COMPLETEPROJECTLOADNOTIFICATION_HPP

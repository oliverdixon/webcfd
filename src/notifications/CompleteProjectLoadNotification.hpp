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
 * @todo Document
 */
class CompleteProjectLoadNotification
{
public:
    explicit CompleteProjectLoadNotification(
            const Project::id_type project_id
    ) :
        project_id(project_id)
    {
    }

    Project::id_type project_id;
};

} // namespace echomap

#endif // ECHOMAP_COMPLETEPROJECTLOADNOTIFICATION_HPP

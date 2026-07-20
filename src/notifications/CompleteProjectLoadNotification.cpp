/**
 * @file
 *
 * CompleteProjectLoadNotification implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-20
 */

#include "CompleteProjectLoadNotification.hpp"

#include "../errors/IgnoredWarning.hpp"

namespace echomap
{

CompleteProjectLoadNotification::CompleteProjectLoadNotification(
        const id_type project_id
) :
    project_id(project_id)
{
}

void CompleteProjectLoadNotification::verify_project(
        const Project* const context
) const
{
    if (context == nullptr)
        throw IgnoredWarning("Dropping CompleteProjectLoadNotification due to empty project.");

    if (context->get_id() != project_id)
        throw IgnoredWarning(
                std::format(
                        "Dropping CompleteProjectLoadNotification due to invalid project: requested {}, but have {}.",
                        project_id,
                        context->get_id()
                )
        );
}

} // namespace echomap

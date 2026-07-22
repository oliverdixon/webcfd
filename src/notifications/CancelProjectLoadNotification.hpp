/**
 * @file
 *
 * CancelProjectLoadNotification specification
 *
 * @author Oliver Dixon
 * @date 2026-07-22
 */

#ifndef ECHOMAP_CANCELPROJECTLOADNOTIFICATION_HPP
#define ECHOMAP_CANCELPROJECTLOADNOTIFICATION_HPP

#include "../objects/IDAllocator.hpp"

namespace echomap
{

class Project;

/**
 * A notification indicating that a pending Project load should be cancelled.
 *
 * Produced by the IndividualUploadModal if the user states a preference to cancel a VFS mapping.
 */
struct CancelProjectLoadNotification
{
    /**
     * Create a new CancelProjectLoadNotification to indicate that the identified Project should be cancelled.
     *
     * @param project_id The ID of the partially loaded Project to cancel.
     */
    explicit CancelProjectLoadNotification(id_type project_id);

    /**
     * Verify that the given Project matches the intended target.
     *
     * @param context The context to which the notification will apply.
     * @throws IgnoredWarning The notification does not apply to the given context and should be ignored.
     */
    void verify_project(const Project* context) const;

    id_type project_id; /**< The ID of the partially loaded Project to cancel. */
};

} // namespace echomap

#endif // ECHOMAP_CANCELPROJECTLOADNOTIFICATION_HPP

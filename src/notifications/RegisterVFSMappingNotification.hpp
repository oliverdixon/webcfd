/**
 * @file
 *
 * RegisterVFSMappingNotification specification
 *
 * @author Oliver Dixon
 * @date 2026-07-19
 */

#ifndef ECHOMAP_REGISTERVFSMAPPINGNOTIFICATION_HPP
#define ECHOMAP_REGISTERVFSMAPPINGNOTIFICATION_HPP

#include <filesystem>

#include "../objects/IDAllocator.hpp"

namespace echomap
{

class Project;

/**
 * A notification to indicate a new VFS mapping from an external source.
 *
 * Produced by the ActionController on RegisterVFSMapping callbacks to indicate a new VFS file mapping has been defined
 * by the user.
 *
 * @ingroup Notifications RegisterVFSMapping
 */
struct RegisterVFSMappingNotification
{
    /**
     * Creates a new RegisterVFSMappingNotification to express a mapping between the external and internal paths.
     *
     * @param project_id The ID of the Project to which the mapping relates.
     * @param external The path on the external (inaccessible) file system as referenced in Project files.
     * @param internal The path on the internal (accessible) VFS corresponding to the external path.
     */
    RegisterVFSMappingNotification(
            id_type project_id,
            std::filesystem::path external,
            std::filesystem::path internal
    );

    /**
     * Verify that the given Project matches the intended target.
     *
     * @param context The context to which the notification will apply.
     * @throws IgnoredWarning The notification does not apply to the given context and should be ignored.
     */
    void verify_project(const Project* context) const;

    id_type project_id;             /**< The ID of the Project to which the mapping relates. */
    std::filesystem::path external; /**< The path on the external file system. */
    std::filesystem::path internal; /**< The path on the internal file system. */
};

} // namespace echomap

#endif // ECHOMAP_REGISTERVFSMAPPINGNOTIFICATION_HPP

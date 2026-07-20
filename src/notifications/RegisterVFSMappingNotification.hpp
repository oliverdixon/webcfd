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

#include "../objects/Project.hpp"

namespace echomap
{

/**
 * A notification to indicate a new VFS mapping from an external source.
 *
 * @ingroup Notifications
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
            const Project::id_type project_id,
            std::filesystem::path external,
            std::filesystem::path internal
    ) :
        project_id(project_id),
        external(std::move(external)),
        internal(std::move(internal))
    {
    }

    Project::id_type project_id;    /**< The ID of the Project to which the mapping relates. */
    std::filesystem::path external; /**< The path on the external file system. */
    std::filesystem::path internal; /**< The path on the internal file system. */
};

} // namespace echomap

#endif // ECHOMAP_REGISTERVFSMAPPINGNOTIFICATION_HPP

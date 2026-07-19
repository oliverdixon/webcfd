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

#include "../../objects/Project.hpp"

namespace echomap
{

/**
 * @todo Document
 */
struct RegisterVFSMappingNotification
{
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

    Project::id_type project_id;

    std::filesystem::path external;
    std::filesystem::path internal;
};

} // namespace echomap

#endif // ECHOMAP_REGISTERVFSMAPPINGNOTIFICATION_HPP

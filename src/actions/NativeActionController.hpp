/**
 * @file
 *
 * NativeActionController specification
 *
 * @author Oliver Dixon
 * @date 2026-07-18
 */

#ifndef ECHOMAP_NATIVEACTIONCONTROLLER_HPP
#define ECHOMAP_NATIVEACTIONCONTROLLER_HPP

#include "ActionControllerBase.hpp"

namespace echomap
{

/**
 * Specialisation of ActionControllerBase to service actions on a native platform.
 *
 * @ingroup Actions
 */
class NativeActionController : public ActionControllerBase<NativeActionController>
{
    friend ActionControllerBase;

    /**
     * Invokes the native function for @ref ProjectFileAction.
     * @implements ActionControllerBase::select_project_file
     * @ingroup ProjectFileAction
     */
    static void select_project_file_impl();

    /**
     * Invokes the native function for @ref RegisterVFSMapping.
     *
     * @param project_id The ID of the Project that owns the destination Signal.
     * @param external The path of the external file being mapped into the VFS.
     *
     * @ingroup RegisterVFSMapping
     * @implements ActionControllerBase::register_vfs_mapping
     */
    static void register_vfs_mapping_impl(
            std::size_t project_id,
            const std::filesystem::path& external
    );
};

} // namespace echomap

#endif // ECHOMAP_NATIVEACTIONCONTROLLER_HPP

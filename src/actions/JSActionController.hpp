/**
 * @file
 *
 * JSActionController specification
 *
 * @author Oliver Dixon
 * @date 2026-07-18
 */

#ifndef ECHOMAP_JSACTIONCONTROLLER_HPP
#define ECHOMAP_JSACTIONCONTROLLER_HPP

#include "ActionControllerBase.hpp"

extern "C" int echomap_on_project_file_picked(const char* path) noexcept;
extern "C" int echomap_on_register_vfs_mapping(
        std::size_t,
        const char*,
        const char*
) noexcept;

namespace echomap
{

/**
 * Specialisation of ActionControllerBase to service actions on a WebAssembly target (using the Emscripten runtime).
 *
 * @ingroup Actions
 */
class JSActionController : public ActionControllerBase<JSActionController>
{
    friend ActionControllerBase;
    friend int ::echomap_on_project_file_picked(const char* path) noexcept;
    friend int ::echomap_on_register_vfs_mapping(
            std::size_t,
            const char*,
            const char*
    ) noexcept;

    /**
     * Invokes the JS function for @ref ProjectFileAction.
     * @ingroup ProjectFileAction
     * @implements ActionControllerBase::select_project_file
     */
    static void select_project_file_impl();

    /**
     * Invokes the JS function for @ref RegisterVFSMapping.
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

    /**
     * Services the callback for the @ref ProjectFileAction.
     *
     * @param path The C++ path of the selected file.
     * @ingroup ProjectFileAction
     * @implements ActionControllerBase::notify_project_file
     */
    static void notify_project_file_impl(const std::filesystem::path& path);

    /**
     * Services the callback for the @ref RegisterVFSMapping.
     *
     * @param project_id The ID of the Project that owns the destination Signal.
     * @param external The path of the external file being mapped into the VFS.
     * @param internal The path of the VFS file.
     *
     * @ingroup RegisterVFSMapping
     * @implements ActionControllerBase::notify_vfs_mapping
     */
    static void notify_vfs_mapping_impl(
            std::size_t project_id,
            const std::filesystem::path& external,
            const std::filesystem::path& internal
    );
};

} // namespace echomap

#endif // ECHOMAP_JSACTIONCONTROLLER_HPP

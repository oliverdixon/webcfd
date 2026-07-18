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
public:
    friend ActionControllerBase;

    /**
     * Invokes the native function for @ref ProjectFileAction.
     * @implements ActionControllerBase::select_project_file
     * @ingroup ProjectFileAction
     */
    static void select_project_file_impl();

    /**
     * Services the callback for the @ref ProjectFileAction.
     *
     * @param path The C++ path of the selected file.
     * @ingroup ProjectFileAction
     * @implements ActionControllerBase::notify_project_file
     */
    static void notify_project_file_impl(const std::filesystem::path& path);
};

} // namespace echomap

#endif // ECHOMAP_NATIVEACTIONCONTROLLER_HPP

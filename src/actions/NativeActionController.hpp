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
     * Invokes the native function for @ref CompletePartialSignalLoad.
     *
     * @param project_id The ID of the Project that owns the destination Signal.
     * @param signal_id The ID of the destination Signal.
     *
     * @ingroup CompletePartialSignalLoad
     * @implements ActionControllerBase::complete_signal_load
     */
    static void complete_signal_load_impl(
            std::size_t project_id,
            std::size_t signal_id
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
     * Services the callback for the @ref CompletePartialSignalLoad.
     *
     * @param project_id The ID of the Project that owns the destination Signal.
     * @param signal_id The ID of the destination Signal.
     * @param path The path derived from the prompt.
     *
     * @ingroup CompletePartialSignalLoad
     * @implements ActionControllerBase::notify_complete_signal_load
     */
    static void complete_signal_load_impl(
            std::size_t project_id,
            std::size_t signal_id,
            const std::filesystem::path& path
    );
};

} // namespace echomap

#endif // ECHOMAP_NATIVEACTIONCONTROLLER_HPP

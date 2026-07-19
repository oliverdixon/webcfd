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
extern "C" int echomap_on_signal_load_complete(
        std::size_t,
        std::size_t,
        const char* path
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
    friend int ::echomap_on_signal_load_complete(
            std::size_t,
            std::size_t,
            const char* path
    ) noexcept;

    /**
     * Invokes the JS function for @ref ProjectFileAction.
     * @ingroup ProjectFileAction
     * @implements ActionControllerBase::select_project_file
     */
    static void select_project_file_impl();

    /**
     * Invokes the JS function for @ref CompletePartialSignalLoad.
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
    static void notify_complete_signal_load_impl(
            std::size_t project_id,
            std::size_t signal_id,
            const std::filesystem::path& path
    );
};

} // namespace echomap

#endif // ECHOMAP_JSACTIONCONTROLLER_HPP

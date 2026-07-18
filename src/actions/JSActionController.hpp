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

    /**
     * Invokes the JS function for @ref ProjectFileAction.
     * @ingroup ProjectFileAction
     * @implements ActionControllerBase::select_project_file
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

#endif // ECHOMAP_JSACTIONCONTROLLER_HPP
